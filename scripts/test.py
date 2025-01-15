import multiprocessing
from multiprocessing.pool import ThreadPool as ThreadPool
import subprocess as sp
import sys
import json
import os
import codecs
from dataclasses import dataclass,fields
from enum import Enum

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
# Need to change the way we are parallelizing the tests. I first want to run all the hash/tokenizer checks before starting a single pc-emul run.
#   At the same time, I do not want to prevent threads from starting to run pc-emul because they are waiting for a hash computation to finish (same thing with sim-run waiting for pc-emul)
#     Before progressing further, need to change Pool to a work based architecture.
#     Besides, using the Pool appears to not solve the problem of left over python executables 
#     
# Can easily add per test information. Something like custom timeouts for commands and stuff like that.
#
# For things like architecture change at the hardware level, it should be something that is done for all the tests at the same time.
#   Our maybe it is better if we do it by group. Things like AXI DATA_W changing does not affect config share and stuff like that, so no point in running those tests for those cases. We want to run the VRead/VWrite tests and stuff like that
#
# Need to catch the exceptions by timeout and report to the user that a timeout occured (and at what stage).
#
# Check if we can run sim-run without 
#
# Can start saving the average amount of time that a test took to run and then use that information to change the order to run the fastest tests first (with more )
#
# Add proper parsing so we can embed comments into the tests
#
# Order of these are important. Assuming that failing sim-run means that passed pc-emul 
# TODO: Maybe make enum, depending on what python enums offer us
class Stage(Enum):
   NOT_WORKING = 0
   PC_EMUL = 1
   SIM_RUN = 2
   FPGA_RUN = 3

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

@dataclass
class TestInfo:
   name: str
   finalStage: Stage
   stage: Stage = Stage.NOT_WORKING
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

@dataclass
class OneTest:
   test: dict
   index: int
   cached: bool
   bestStageReached: Stage
   error: bool = False
   tokens: int = 0
   hashVal: int = 0

def GetOrDefault(d,name,default):
   if name in d:
      return d[name]
   else:
      d[name] = default
      return default

def ProcessOneTest(testWithIndex):
   index = testWithIndex[0]
   test = testWithIndex[1]
   name = test['name']

   testTempDir = TempDir(name)

   stage = Stage.NOT_WORKING
   finalStage = Stage.NOT_WORKING
   try:
      stage = Stage[test.get('stage',"NOT_WORKING")]
      finalStage = Stage[test.get('finalStage',"NOT_WORKING")]
   except Exception as e:
      print("Error on Process One Test, probably a comment embedded in a enum section")
      print("Need to properly parse stuff, otherwise no problem")
      stage = Stage.NOT_WORKING
      finalStage = Stage.NOT_WORKING

   # TODO: Maybe make a distinction between not working or disabled?
   #       Do we run Versat for not working or is this return good?
   if(finalStage == Stage.NOT_WORKING):
      return OneTest(test,index,False,stage,True)

   versatResult = RunVersat(name,testTempDir,testInfoJson["default_args"])
   if(versatResult.error or SIMULATE):
      return OneTest(test,index,False,stage,True)

   tokenAmount,hashVal = ComputeFilesTokenSizeAndHash(versatResult.filepaths)

   testTokens = test.get('tokens',0)
   testHashVal = test.get('hash',0)

   if(tokenAmount == testTokens and hashVal == testHashVal):
      return OneTest(test,index,True,stage)

   previousReachedStage = stage
   testLastStage = finalStage
   bestStageReached = Stage.NOT_WORKING

   tokens = 0
   hashVal = 0

   for stageNumber in range(1,testLastStage.value + 1):
      stage = Stage(stageNumber)
      returnCode,tokens,hashVal = PerformTest(test['name'],stage)
      passed = (returnCode == 0)

      if(passed):
         bestStageReached = stage

      if(not passed):
         break

   return OneTest(test,index,False,bestStageReached,False,tokens,hashVal)

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
   print("Gonna start running tests")

   if(command == "reset"):
      for i in range(0,len(testInfoJson['tests'])):
         testInfoJson['tests'][i] = {
            'name' : testInfoJson['tests'][i]['name'],
            'finalStage' : testInfoJson['tests'][i].get('finalStage',Stage.NOT_WORKING)
         }

      with open(jsonfilePath,"w") as file:
         json.dump(testInfoJson,file,cls=MyJsonEncoder,indent=2)

      sys.exit(0)

   # TODO: Pool waits until everything completes before producing output. Not a big fan.
   #       Rewrite to a work module that produces output as soon as possible.
   #       The writting of the json can be the last thing done, just the output to the terminal is the thing that needs to occur fast
   testResults = None

   # Threads seem as fast as processes, maybe if we add more python logic this changes but for now keep threads. Processes was leaving some processes hanging, threads seem fine
   #with multiprocessing.Pool(16) as p:
   with ThreadPool(16) as p:
      testResults = p.map(ProcessOneTest,enumerate(testInfoJson['tests']))

   def GeneratePad(word,amount,padding = '.'):
      return padding * (amount - len(word))

   testsOutput = []
   # We want output as fast as possible, so the report part of this code should move to the multiprocessing
   for index,result in enumerate(testResults):
      test = result.test
      name = test['name']

      finalStage = Stage.NOT_WORKING
      try:
         finalStage = Stage[test.get('finalStage',"NOT_WORKING")]
      except Exception as e:
         print("Error on Process One Test, probably a comment embedded in a enum section")
         print("Need to properly parse stuff, otherwise no problem")
         finalStage = Stage.NOT_WORKING

      stage = result.bestStageReached

      padding = ('.' if index % 2 == 0 else '-')

      testName = name
      versatFail = "VERSAT_FAIL"
      failing = "FAIL"
      partial = "PARTIAL"
      ok = "OK"
      disabled = "DISABLED"
      cached = "(cached)" if result.cached else ""

      condition = ok

      color = COLOR_GREEN
      if(finalStage == Stage.NOT_WORKING):
         condition = disabled
         color = COLOR_GREEN
      elif(stage == Stage.NOT_WORKING):
         condition = failing
         color = COLOR_RED
      elif(stage != finalStage):
         color = COLOR_YELLOW
         condition = partial

      if(result.error):
         color = COLOR_RED
         testsOutput.append((testName,versatFail,cached,color))
      else:
         testsOutput.append((testName,condition,cached,color))

   firstColumnSize = max([len(x[0]) for x in testsOutput]) + 1
   secondColumnSize = max([len(x[1]) for x in testsOutput])

   for output in testsOutput:
      first = output[0]
      second = output[1]
      cached = output[2]
      color = output[3]

      firstPad = GeneratePad(first,firstColumnSize)
      secondPad = GeneratePad(second,secondColumnSize) 

      print(f"{first}{firstPad}{secondPad}{color}{second}{COLOR_BASE}{cached}")

   if(SIMULATE):
      sys.exit(0)

   for result in testResults:
      if(not result.error and not result.cached and not SIMULATE):
         testInfoJson['tests'][result.index]['tokens'] = result.tokens
         testInfoJson['tests'][result.index]['hash'] = result.hashVal
         testInfoJson['tests'][result.index]['stage'] = stage.name

   if doUpdate:
      with open(jsonfilePath,"w") as file:
         json.dump(testInfoJson,file,cls=MyJsonEncoder,indent=2)
