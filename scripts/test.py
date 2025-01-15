import multiprocessing
import subprocess as sp
import sys
import json
import os
import codecs
from dataclasses import dataclass,fields
from enum import Enum

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
      result = sp.run(args,capture_output=True)
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
      result = sp.run(args,capture_output=True)
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
      result = sp.run(command,capture_output=True,shell=True)
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

      filepathList = FindAndParseFilepathList(output)
      tokens,hashVal = ComputeFilesTokenSizeAndHash(filepathList)

      return returnCode,tokens,hashVal
   if stage == Stage.SIM_RUN:
      returnCode,output = RunMakefile("sim-run",test)

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

   stage = Stage[test.get('stage',"NOT_WORKING")]
   finalStage = Stage[test.get('finalStage',"NOT_WORKING")]

   versatResult = RunVersat(name,testTempDir,testInfoJson["default_args"])
   if(versatResult.error):
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

   if(command == "reset"):
      with open(jsonfilePath,"r") as file:
         try:
            testInfoJson = json.load(file)
         except Exception as e:
            print(f"Failed to parse testInfo file: {e}")
            sys.exit(0)

      for i in range(0,len(testInfoJson['tests'])):
         testInfoJson['tests'][i] = {
            'name' : testInfoJson['tests'][i]['name'],
            'finalStage' : testInfoJson['tests'][i].get('finalStage',Stage.NOT_WORKING)
         }

      with open(jsonfilePath,"w") as file:
         json.dump(testInfoJson,file,cls=MyJsonEncoder,indent=2)

      sys.exit(0)

   with open(jsonfilePath,"r") as file:
      try:
         testInfoJson = json.load(file)
      except Exception as e:
         print(f"Failed to parse testInfo file: {e}")
         sys.exit(0)

   # TODO: Pool waits until everything completes before producing output. Not a big fan.
   #       Rewrite to a work module that produces output as soon as possible.
   #       The writting of the json can be the last thing done, just the output to the terminal is the thing that needs to occur fast
   testResults = None
   with multiprocessing.Pool(8) as p:
      testResults = p.map(ProcessOneTest,enumerate(testInfoJson['tests']))

   def PadLeft(word,amount):
      if(amount < len(word)):
         return word[:amount]
      res = '.' * (amount - len(word)) + word
      return res

   def PadRight(word,amount):
      if(amount < len(word)):
         return word[:amount]
      res = word + '.' * (amount - len(word))
      return res

   firstColumnSize = 50
   secondColumnSize = 10
   thirdColumnSize = 60

   # We want output as fast as possible, so the report part of this code should move to the multiprocessing
   for result in testResults:
      test = result.test
      name = test['name']
      finalStage = Stage[test.get('finalStage',"NOT_WORKING")]
      stage = result.bestStageReached

      testName = PadRight(name,firstColumnSize)
      cached = PadRight("cached" if result.cached else "",secondColumnSize)

      versatFail = PadLeft(f"{COLOR_RED}VERSAT_FAIL{COLOR_BASE}",thirdColumnSize)
      failing = PadLeft(f"{COLOR_RED}FAIL{COLOR_BASE}",thirdColumnSize)
      partial = PadLeft(f"{COLOR_YELLOW}PARTIAL{COLOR_BASE}({stage.name}/{finalStage.name})",thirdColumnSize)
      ok = PadLeft(f"{COLOR_GREEN}OK{COLOR_BASE}",thirdColumnSize)
      disabled = PadLeft(f"{COLOR_GREEN}DISABLED{COLOR_BASE}",thirdColumnSize)

      condition = ok

      if(finalStage == Stage.NOT_WORKING):
         condition = disabled
      elif(stage == Stage.NOT_WORKING):
         condition = failing
      elif(stage != finalStage):
         condition = partial

      if(result.error):
         print(f"{testName}{cached}{versatFail}")
      else:
         print(f"{testName}{cached}{condition}")

      if(not result.error and not result.cached):
         testInfoJson['tests'][result.index]['tokens'] = result.tokens
         testInfoJson['tests'][result.index]['hash'] = result.hashVal
         testInfoJson['tests'][result.index]['stage'] = stage.name

   if doUpdate:
      with open(jsonfilePath,"w") as file:
         json.dump(testInfoJson,file,cls=MyJsonEncoder,indent=2)
