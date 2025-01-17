import multiprocessing
from multiprocessing.pool import ThreadPool as ThreadPool
import subprocess as sp
import threading
import sys
import json
import os
import codecs
import queue
import random
from dataclasses import dataclass,fields
from enum import Enum,auto

SIMULATE = False

##############
# TODO: Add a brief description here of what we are doing
# Bunch of stuff that was supposed to be on Json, but since it does not support comments, put it here.
# Basically, just a big warning for future maintainers to not touch certain parts of the test stage unless they know what they are doing
##############

# TODO:
# Save the text/output of the tests that fail. The stdout from running the tests and stuff
# Add the .cpp file to the tokenizer and hasher otherwise we run the risk of having false results for tests that only have software problems.
# Add some commands to the tool that allow us to force stuff, like forcing the tokens/hash of a bad test case and stuff like that.
# Need to provide some form of progress reporting on the terminal
#   Could be something as simple as a print that uses \r to update and that displays how many tests are running, how many compiled correctly, how many passed pc-emul and so on.
# Maybe useful to group tests that are similar into groups and take groups into account when outputting stuff.
#     
# Can easily add per test information. Something like custom timeouts for commands and stuff like that.
# Can also start saving some info regarding the tests itself, like the average amount of time spend per versat and stuff like that.
#
# For things like architecture change at the hardware level, it should be something that is done for all the tests at the same time.
#   Our maybe it is better if we do it by group. Things like AXI DATA_W changing does not affect config share and stuff like that, so no point in running those tests for those cases. We want to run the VRead/VWrite tests and stuff like that
#
# Need to catch the exceptions by timeout and report to the user that a timeout occured (and at what stage).
#
# Check if we can run sim-run without doing a make clean first
#

# Order of these are important. Assuming that failing sim-run means that passed pc-emul 
class Stage(Enum):
   DISABLED = auto()
   NOT_WORKING = auto()
   PC_EMUL = auto()
   SIM_RUN = auto()
   FPGA_RUN = auto()

COLOR_BASE   = '\33[0m'
COLOR_RED    = '\33[31m'
COLOR_GREEN  = '\33[32m'
COLOR_YELLOW = '\33[33m'
COLOR_BLUE   = '\33[34m'
COLOR_MAGENTA= '\33[35m'
COLOR_CYAN   = '\33[36m'
COLOR_WHITE  = '\33[37m'

@dataclass
class JsonContent:
   default_args: str = ""
   tests : list = None

   def __init__(self,jsonElem):
      fieldNames = [x.name for x in fields(self)]

      for name in jsonElem:
         if not name in fieldNames:
            print(f"Error, Json contains a none used keyword: {name}")
            sys.exit(0)

      self.__dict__ = jsonElem

def DefaultStage():
   return Stage.NOT_WORKING

@dataclass
class TestInfo:
   name: str
   finalStage: Stage
   stage: Stage = DefaultStage()
   tokens: int = 0
   hashVal: int = 0

   def __init__(self,jsonElem):
      fieldNames = [x.name for x in fields(self)]

      for name in jsonElem:
         if not name in fieldNames:
            print(f"Error, Json contains a none used keyword: {name}")
            sys.exit(0)

      self.__dict__ = jsonElem

class MyJsonEncoder(json.JSONEncoder):
   def default(self,o):
      if(type(o) == Stage):
         return o.name
      else:
         return super().default(o)

@dataclass
class VersatResult:
   error: bool
   filepaths: list

def FindAndParseFilepathList(content):
   # Probably bottleneck
   filePathList = []
   for line in content.split("\n"):
      tokens = line.split(" ")

      fileName = None
      fileType = None

      if(len(tokens) >= 2):
         if(tokens[0] == "Filename:"):
            fileName = tokens[1]
      if(len(tokens) >= 4):
         if(tokens[2] == "Type:"):
            fileType = tokens[3]

      if(fileName):
         filePathList.append(fileName)

   return filePathList

def RunVersat(testName,testFolder,versatExtra):
   args = ["./submodules/VERSAT/versat"]

   args += ["./versatSpec.txt"]
   args += ["-O",f"{testFolder}/software"]
   args += ["-o",f"{testFolder}/hardware"]
   args += ["-t",f"{testName}"]
   args += versatExtra.split(" ")

   result = None
   try:
      result = sp.run(args,capture_output=True,timeout=10) # Maybe a bit low for merge based tests, eventually add timeout 'option' to the test itself
   except Exception as e:
      print(f"Except on calling Versat:{e}") # This should not happen
      return VersatResult(True,[])

   returnCode = result.returncode
   #errorOutput = codecs.getdecoder("unicode_escape")(result.stderr)[0]
   output = codecs.getdecoder("unicode_escape")(result.stdout)[0]

   if(returnCode != 0):
      return VersatResult(True,[])

   filePathList = FindAndParseFilepathList(output)

   # Parse result.
   return VersatResult(False,filePathList)

# TODO: Put this in an actual temp folder
def TempDir(testName):
   return f"./testCache/{testName}"

def ComputeFilesTokenSizeAndHash(files):
   args = ["./submodules/VERSAT/build/calculateHash"] + files

   result = None
   try:
      result = sp.run(args,capture_output=True,timeout=5)
   except Exception as e:
      print(f"Except on ComputeHash:{e}")
      return -1,-1

   returnCode = result.returncode
   output = codecs.getdecoder("unicode_escape")(result.stdout)[0]
   #errorOutput = codecs.getdecoder("unicode_escape")(result.stderr)[0]

   if(returnCode == 0):
      amountOfTokens,hashVal = [int(x) for x in output.split(":")]

      return (amountOfTokens,hashVal)
   else:
      return -1,-1

# Probably do not want to use makefile, but for now...
def RunMakefile(target,testName):
   result = None
   try:
      args = ["make",target,f"TEST={testName}"]

      command = f"make clean pc-emul-run TEST={testName}"
      result = sp.run(command,capture_output=True,shell=True,timeout=60) # 60
   # TODO: Add proper error propragation information so we know we time outted.
   #except subprocess.TimeoutExpired as t:
   #   return -1,""
   except Exception as e:
      print(f"Except on calling makefile:{e}")
      return -1,""

   returnCode = result.returncode
   errorOutput = codecs.getdecoder("utf-8")(result.stderr)[0]
   #output = codecs.getdecoder("utf-8")(result.stdout)[0]

   return returnCode,errorOutput

# Need to not only do the test but also get the list of the files generated from versat in order to perform the comparation
def PerformTest(test,stage):
   if stage == Stage.PC_EMUL:
      returnCode,output = RunMakefile("pc-emul-run",test)

      if(returnCode != 0):
         return returnCode,0,0

      filepathList = FindAndParseFilepathList(output)
      tokens,hashVal = ComputeFilesTokenSizeAndHash(filepathList)

      return returnCode,tokens,hashVal
   if stage == Stage.SIM_RUN:
      returnCode,output = RunMakefile("sim-run",test)

      if(returnCode != 0):
         return returnCode,0,0

      filepathList = FindAndParseFilepathList(output)
      tokens,hashVal = ComputeFilesTokenSizeAndHash(filepathList)

      return returnCode,tokens,hashVal
   if stage == Stage.FPGA_RUN:
      # Not implemented yet, 
      return 0,0,0

   print(f"Error, PerformTest called with: {stage}. Fix this")

class WorkState(Enum):
   INITIAL = auto()
   PROCESS = auto()
   FINISH = auto()

@dataclass
class ThreadWork:
   test: dict
   index: int
   cached: bool = False
   error: bool = False
   versatFailed: bool = False
   tokens: int = 0
   hashVal: int = 0
   workStage: WorkState = WorkState.INITIAL
   stageToProcess: Stage = Stage.PC_EMUL
   lastStageReached: Stage = Stage.NOT_WORKING
   
def GetOrDefault(d,name,default):
   if name in d:
      return d[name]
   else:
      d[name] = default
      return default

def GetTestFinalStage(test):
   try:
      content = test['finalStage']
      splitted = content.split(" ")
      return Stage[splitted[0]]
   except:
      return DefaultStage()

def AdvanceOneTest(test):
   index = test.index
   test = testWithIndex[1]
   name = test['name']

   stage = Stage[test.get('stage',DefaultStage().name)]
   finalStage = GetTestFinalStage(test)

def PrintResult(result,firstColumnSize):
   def GeneratePad(word,amount,padding = '.'):
      return padding * (amount - len(word))

   test = result.test
   name = test['name']

   finalStage = GetTestFinalStage(test)
   stage = result.lastStageReached

   testName = name
   isError = result.error
   versatFail = "VERSAT_FAIL"
   failing = "FAIL"
   partial = f"PARTIAL"
   partialVal = ""
   ok = "OK"
   disabled = "DISABLED"
   cached = "(cached)" if result.cached else ""

   condition = ok

   color = COLOR_GREEN
   if(finalStage == Stage.DISABLED):
      condition = disabled
      color = COLOR_YELLOW
      isError = False
   elif(stage == Stage.NOT_WORKING):
      condition = failing
      color = COLOR_RED
   elif(stage != finalStage):
      color = COLOR_YELLOW
      condition = partial
      partialVal = f"[{stage.name}/{finalStage.name}]"

   if(isError):
      color = COLOR_RED
      partialVal = ""
      condition = versatFail

   firstPad = GeneratePad(testName,firstColumnSize)
   secondPad = GeneratePad(condition,1)
   print(f"{testName}{firstPad}{secondPad}{color}{condition}{COLOR_BASE}{partialVal}{cached}")

# This does everything. We want to do stuff as we go along.
def ProcessOneTest(testWithIndex):
   index = testWithIndex[0]
   test = testWithIndex[1]
   name = test['name']

   stage = Stage[test.get('stage',DefaultStage().name)]
   finalStage = GetTestFinalStage(test)

   if(SIMULATE):
      return OneTest(test,index,random.choice([True,False]),random.choice(list(Stage)),random.choice([True,False]))

   if(finalStage == Stage.DISABLED):
      return OneTest(test,index,False,stage,True)

   testTempDir = TempDir(name)

   versatResult = RunVersat(name,testTempDir,testInfoJson["default_args"])
   if(versatResult.error):
      return OneTest(test,index,False,stage,True)

   tokenAmount,hashVal = ComputeFilesTokenSizeAndHash(versatResult.filepaths)

   testTokens = test.get('tokens',0)
   testHashVal = test.get('hash',0)

   if(tokenAmount == testTokens and hashVal == testHashVal):
      return OneTest(test,index,True,stage,False,testTokens,testHashVal)

   previousReachedStage = stage
   testLastStage = finalStage
   lastStageReached = DefaultStage()

   tokens = 0
   hashVal = 0

   for stageNumber in range(Stage.PC_EMUL.val,testLastStage.value + 1):
      stage = Stage(stageNumber)
      returnCode,tokens,hashVal = PerformTest(test['name'],stage)
      passed = (returnCode == 0)

      if(passed):
         lastStageReached = stage
      else:
         break

   return OneTest(test,index,False,lastStageReached,False,tokens,hashVal)

def ProcessWork(work):
   test = work.test
   name = test['name']

   finalStage = GetTestFinalStage(test)

   if(work.workStage == WorkState.INITIAL):
      if(finalStage == Stage.DISABLED):
         work.workStage = WorkState.FINISH
         work.lastStageReached = finalStage
         return work

      testTempDir = TempDir(name)

      versatResult = RunVersat(name,testTempDir,testInfoJson["default_args"])
      if(versatResult.error):
         work.error = True
         work.versatFailed = True
         work.workStage = WorkState.FINISH
         return work

      tokenAmount,hashVal = ComputeFilesTokenSizeAndHash(versatResult.filepaths)
      testTokens = test.get('tokens',0)
      testHashVal = test.get('hash',0)

      work.tokens = tokenAmount
      work.hashVal = hashVal

      if(tokenAmount == testTokens and hashVal == testHashVal):
         work.cached = True
         work.lastStageReached = Stage[test.get('stage')]
         work.workStage = WorkState.FINISH
         return work

      work.workStage = WorkState.PROCESS
      work.stageToProcess = Stage.PC_EMUL
      return work
   elif(work.workStage == WorkState.PROCESS):
      stage = Stage(work.stageToProcess)
      returnCode,tokens,hashVal = PerformTest(test['name'],stage)
      passed = (returnCode == 0)

      if(passed):
         work.lastStageReached = stage
         
         if(work.lastStageReached == finalStage):
            work.workStage = WorkState.FINISH
         else:
            work.stageToProcess = Stage(work.stageToProcess.value + 1)
      else:
         work.error = True

      return work

def ThreadMain(workQueue,resultQueue,index):
   while(True):
      work = workQueue.get()
      
      if(work == "Exit"):
         break
      else:
         #print(work)
         result = ProcessWork(work)
         #print(result)
         resultQueue.put(result)

if __name__ == "__main__":
   testInfoJson = None
   command = sys.argv[1]
   jsonfilePath = sys.argv[2]

   doUpdate = (command != "run-only") 
   SIMULATE = (command == "simulate")

   with open(jsonfilePath,"r") as file:
      try:
         testInfoJson = json.load(file)
      except Exception as e:
         print(f"Failed to parse testInfo file: {e}")
         sys.exit(0)

   allTestNames = [x['name'] for x in testInfoJson['tests']]

   nameCount = {}
   for name in allTestNames:
      nameCount[name] = nameCount.get(name,0) + 1

   repeatedElements = [x for x,y in nameCount.items() if y != 1]
   if(len(repeatedElements)):
      print("There are repeated tests:\n\t","\t\n".join(repeatedElements),sep='')
      print("Exiting. Fix test info and run again.")
      sys.exit(0)

   # Put any check to the data above this line. # From this point assume data is correct

   if(command == "reset"):
      for i in range(0,len(testInfoJson['tests'])):
         testInfoJson['tests'][i] = {
            'name' : testInfoJson['tests'][i]['name'],
            'finalStage' : testInfoJson['tests'][i].get('finalStage',DefaultStage().name)
         }

      with open(jsonfilePath,"w") as file:
         json.dump(testInfoJson,file,cls=MyJsonEncoder,indent=2)

      sys.exit(0)

   print("Gonna start running tests")

   # TODO: Pool waits until everything completes before producing output. Not a big fan.
   #       Rewrite to a work module that produces output as soon as possible.
   #       The writting of the json can be the last thing done, just the output to the terminal is the thing that needs to occur fast
   testResults = None

   # Threads seem as fast as processes, maybe if we add more python logic this changes but for now keep threads. Processes was leaving some processes hanging, threads seem fine
   #with multiprocessing.Pool(16) as p:

   amountOfThreads = 8

   workQueue = queue.Queue()
   resultQueue = queue.Queue()
   threadList = [threading.Thread(target=ThreadMain,args=[workQueue,resultQueue,x],daemon=True) for x in range(amountOfThreads)]
   for thread in threadList:
      thread.start()

   amountOfWork = 0
   for index,test in enumerate(testInfoJson['tests']):
      work = ThreadWork(test,index)

      amountOfWork += 1
      workQueue.put(work)

   maxNameLength = max([len(test['name']) for test in testInfoJson['tests']]) + 1

   while(amountOfWork > 0):
      result = resultQueue.get()

      #sys.exit()

      amountOfWork -= 1
      if(result.workStage == WorkState.FINISH):
         PrintResult(result,maxNameLength)

         if(not result.error and not SIMULATE):
            testInfoJson['tests'][result.index]['tokens'] = result.tokens
            testInfoJson['tests'][result.index]['hash'] = result.hashVal
            testInfoJson['tests'][result.index]['stage'] = result.lastStageReached.name

      else:
         amountOfWork += 1
         workQueue.put(result)

   # Since we are exitting soon, we could skip this, but for now lets see if we can catch some bugs this way
   for x in range(amountOfThreads):
      workQueue.put("Exit")

   for thread in threadList:
      thread.join() 

   sys.exit()

   if(SIMULATE):
      sys.exit(0)

   if doUpdate:
      with open(jsonfilePath,"w") as file:
         json.dump(testInfoJson,file,cls=MyJsonEncoder,indent=2)
