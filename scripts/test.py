import subprocess as sp
import threading
import sys
import json
import codecs
import queue
import time
import traceback
from dataclasses import dataclass
from enum import Enum,auto

SIMULATE = False

##############
# TODO: Add a brief description here of what we are doing
# Bunch of stuff that was supposed to be on Json, but since it does not support comments, put it here.
# Basically, just a big warning for future maintainers to not touch certain parts of the test stage unless they know what they are doing
##############

# TODO:
# Refactor commands if adding more commands. Maybe add argparse and work from there.
# Save the text/output of the tests that fail. The stdout from running the tests and stuff
# Add some commands to the tool that allow us to force stuff, like forcing the tokens/hash of a bad test case and stuff like that.
# Maybe useful to group tests that are similar into groups and take groups into account when outputting stuff.
#  Only consider this if we start having a huge amount of tests.
# Can easily add per test information. Something like custom timeouts for commands and stuff like that.
# Can also start saving some info regarding the tests itself, like the average amount of time spend per versat and stuff like that.
#
# For things like architecture change at the hardware level, it should be something that is done for all the tests at the same time.
#   Our maybe it is better if we do it by group. Things like AXI DATA_W changing does not affect config share and stuff like that, so no point in running those tests for those cases. We want to run the VRead/VWrite tests and stuff like that
#
# Need to catch the exceptions by timeout and report to the user that a timeout occured (and at what stage).
#
# Check if we can run sim-run without doing a clean first
#
# The tests right now run from make which causes nix-shell to get called all the time.
#    It's better to just have the one time nix-shell call when starting the test.py 

# Order of these are important. Assuming that failing sim-run means that it passed pc-emul 
class Stage(Enum):
   DISABLED = auto()
   TEMP_DISABLED = auto()
   NOT_WORKING = auto()
   PC_EMUL = auto()
   SIM_RUN = auto()
   FPGA_RUN = auto()

class ErrorType(Enum):
   NONE = auto()
   EXCEPT = auto()
   TIMEOUT = auto()
   PROGRAM_ERROR = auto()

class ErrorSource(Enum):
   NO_SOURCE = auto()
   VERSAT = auto()
   HASHER = auto()
   MAKEFILE = auto()

@dataclass
class Error():
   error: ErrorType = ErrorType.NONE
   source: ErrorSource = ErrorSource.NO_SOURCE

def IsError(errorType):
   if(type(errorType) == Error):
      return IsError(errorType.error)

   assert(type(errorType) == ErrorType) 
   res = (errorType != ErrorType.NONE)
   return res

def DefaultStage():
   return Stage.NOT_WORKING

def IsStageDisabled(stage):
   assert(type(stage) == Stage) 
   res = (stage == Stage.DISABLED or stage == Stage.TEMP_DISABLED)
   return res

COLOR_BASE   = '\33[0m'
COLOR_RED    = '\33[31m'
COLOR_GREEN  = '\33[32m'
COLOR_YELLOW = '\33[33m'
COLOR_BLUE   = '\33[34m'
COLOR_MAGENTA= '\33[35m'
COLOR_CYAN   = '\33[36m'
COLOR_WHITE  = '\33[37m'

class MyJsonEncoder(json.JSONEncoder):
   def default(self,o):
      if(type(o) == Stage):
         return o.name
      else:
         return super().default(o)

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
   except TimeoutExpired as t:
      return Error(ErrorType.TIMEOUT,ErrorSource.VERSAT),[]      
   except Exception as e:
      print(f"Except on calling Versat:{e}") # This should not happen
      return Error(ErrorType.EXCEPT,ErrorSource.VERSAT),[]

   returnCode = result.returncode
   output = codecs.getdecoder("unicode_escape")(result.stdout)[0]

   if(returnCode != 0):
      return Error(ErrorType.PROGRAM_ERROR,ErrorSource.VERSAT),[]

   filePathList = FindAndParseFilepathList(output)

   # Parse result.
   return Error(),filePathList

# TODO: Put this in an actual temp folder
def TempDir(testName):
   return f"./testCache/{testName}"

def ComputeFilesTokenSizeAndHash(files):
   args = ["./submodules/VERSAT/build/calculateHash"] + files

   result = None
   try:
      result = sp.run(args,capture_output=True,timeout=5)
   except TimeoutExpired as t:
      return Error(ErrorType.TIMEOUT,ErrorSource.HASHER),-1,-1
   except Exception as e:
      print(f"Except on ComputeHash:{e}")
      return Error(ErrorType.EXCEPT,ErrorSource.HASHER),-1,-1

   returnCode = result.returncode
   output = codecs.getdecoder("unicode_escape")(result.stdout)[0]
   errorOutput = codecs.getdecoder("unicode_escape")(result.stderr)[0]

   if(returnCode == 0):
      amountOfTokens,hashVal = [int(x) for x in output.split(":")]

      return Error(),amountOfTokens,hashVal
   else:
      return Error(ErrorType.PROGRAM_ERROR,ErrorSource.HASHER),-1,-1

# Probably do not want to use makefile, but for now...
def RunMakefile(target,testName):
   result = None
   try:
      command = " ".join(["make",target,f"TEST={testName}"])

      result = sp.run(command,capture_output=True,shell=True,timeout=60) # 60
   except TimeoutExpired as t:
      return Error(ErrorType.TIMEOUT,ErrorSource.MAKEFILE) 
   except Exception as e:
      print(f"Except on calling makefile:{e}")
      return Error(ErrorType.EXCEPT,ErrorSource.MAKEFILE)

   returnCode = result.returncode
   errorOutput = codecs.getdecoder("utf-8")(result.stderr)[0]

   return Error()

# Need to not only do the test but also get the list of the files generated from versat in order to perform the comparation
def PerformTest(test,stage):
   # This function was previously taking the output from the makefile and checking the files using the hasher.
   # This was done because there might be changes from the sim-run and the pc-emul files (stuff like 32bit vs 64 bit addresses and stuff like that)
   # (Although must of the changes right now are "abstracted" inside the verilator makefile, so the hardware is the same (or should be the same))
   # Regardless. If we eventually start making pc-emul and sim-run different, we need to start calculating the hash for each type (pc-emul vs sim-run)
   # Only handle this case when we need it.

   if stage == Stage.PC_EMUL:
      error = RunMakefile("pc-emul-run",test)

      return error
   if stage == Stage.SIM_RUN:
      error = RunMakefile("sim-run",test)

      return error
   if stage == Stage.FPGA_RUN:
      # Not implemented yet, 
      return Error()

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
   error: Error = Error()
   tokens: int = 0
   hashVal: int = 0
   workStage: WorkState = WorkState.INITIAL
   stageToProcess: Stage = Stage.PC_EMUL
   lastStageReached: Stage = Stage.NOT_WORKING
   
def GetTestFinalStage(test):
   try:
      content = test['finalStage']
      splitted = content.split(" ")
      return Stage[splitted[0]]
   except:
      return DefaultStage()

def PrintResult(result,firstColumnSize):
   def GeneratePad(word,amount,padding = '.'):
      return padding * (amount - len(word))

   test = result.test
   name = test['name']

   finalStage = GetTestFinalStage(test)
   stage = result.lastStageReached

   testName = name
   isError = IsError(result.error)
   failing = "FAIL"
   partial = f"PARTIAL"
   partialVal = ""
   ok = "OK"
   disabled = "DISABLED"
   cached = "(cached)" if result.cached else ""

   condition = ok

   color = COLOR_GREEN
   if(IsStageDisabled(finalStage)):
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
      condition = f"{result.error.error.name}:{result.error.source.name}"

   firstPad = GeneratePad(testName,firstColumnSize)
   secondPad = GeneratePad(condition,1)
   print(f"{testName}{firstPad}{secondPad}{color}{condition}{COLOR_BASE}{partialVal}{cached}")

def CppLocation(test):
   name = test['name']
   return f"./software/src/Tests/{name}.cpp"   

def ProcessWork(work):
   test = work.test
   name = test['name']

   finalStage = GetTestFinalStage(test)

   if(work.workStage == WorkState.INITIAL):
      assert not IsStageDisabled(finalStage)

      testTempDir = TempDir(name)

      versatError,filepaths = RunVersat(name,testTempDir,testInfoJson["default_args"])
      if(IsError(versatError)):
         work.error = versatError
         work.versatFailed = True
         work.workStage = WorkState.FINISH
         return work

      sourceLocation = CppLocation(test)
      filepathsToHash = filepaths + [sourceLocation]
      hashError,tokenAmount,hashVal = ComputeFilesTokenSizeAndHash(filepathsToHash)
      if(IsError(hashError)):
         work.error = hashError
         work.errorSource = ErrorSource.HASHER
         return work

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
      stage = work.stageToProcess
      error = PerformTest(test['name'],stage)
      passed = (not IsError(error))

      if(passed):
         work.lastStageReached = stage
         
         if(work.lastStageReached == finalStage):
            work.workStage = WorkState.FINISH
         else:
            work.stageToProcess = Stage(work.stageToProcess.value + 1)
      else:
         work.error = error

      return work

def ThreadMain(workQueue,resultQueue,index):
   try:
      while(True):
         work = workQueue.get()
         
         if(work == "Exit"):
            break
         else:
            result = ProcessWork(work)
            resultQueue.put(result)
            workQueue.task_done()

   except Exception as e:
      print(f"Exception reached ThreadMain:")
      traceback.print_exception(e)

def RunTests(testInfoJson):
   amountOfThreads = 8
   amountOfTests = len(testInfoJson['tests'])

   workQueue = queue.Queue()
   resultQueue = queue.Queue()
   threadList = [threading.Thread(target=ThreadMain,args=[workQueue,resultQueue,x],daemon=True) for x in range(amountOfThreads)]
   for thread in threadList:
      thread.start()

   maxNameLength = max([len(test['name']) for test in testInfoJson['tests']]) + 1

   amountOfWork = 0
   for index,test in enumerate(testInfoJson['tests']):
      work = ThreadWork(test,index)

      finalStage = GetTestFinalStage(test)
      if(IsStageDisabled(finalStage)):
         PrintResult(work,maxNameLength)
         continue

      amountOfWork += 1
      workQueue.put(work)

   resultList = []

   # Block until all threads finish processing 
   #workQueue.join()

   while(amountOfWork > 0):
      try:
         result = resultQueue.get(True,1)
      except queue.Empty as e:
         time.sleep(1)
         continue

      amountOfWork -= 1

      if(IsError(result.error) or result.workStage == WorkState.FINISH):
         PrintResult(result,maxNameLength)

         resultList.append(result)
      else:
         amountOfWork += 1
         workQueue.put(result)

   # For now, we do not wait for threads to exit. We just keep them waiting for work forever
   #for x in range(amountOfThreads):
   #   workQueue.put("Exit")

   #for thread in threadList:
   #   thread.join() 

   return resultList

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

   resultList = None
   if(command == "run" or command == "run-only" or command == "disable-failed"):
      resultList = RunTests(testInfoJson)

      if(command == 'run'):
         for result in resultList:
            if(not IsError(result.error) and not SIMULATE):
               testInfoJson['tests'][result.index]['tokens'] = result.tokens
               testInfoJson['tests'][result.index]['hash'] = result.hashVal
               testInfoJson['tests'][result.index]['stage'] = result.lastStageReached.name

   if(command == "reenable"):
      for index,test in enumerate(testInfoJson['tests']):
         finalStage = test['finalStage']
         splitted = finalStage.split(" ")
         if(splitted[0] == "TEMP_DISABLED"):
            trueStage = splitted[2].split(":")[1]
            testInfoJson['tests'][index]['finalStage'] = trueStage

   if(command == "disable-failed"):
      doUpdate = True
      for result in resultList:
         if(IsError(result.error)):
            finalStage = GetTestFinalStage(result.test)
            testInfoJson['tests'][result.index]['finalStage'] = f"TEMP_DISABLED - WAS:{finalStage.name}"

   #sys.exit()

   #if(SIMULATE):
   #   sys.exit(0)

   if doUpdate:
      with open(jsonfilePath,"w") as file:
         json.dump(testInfoJson,file,cls=MyJsonEncoder,indent=2)
