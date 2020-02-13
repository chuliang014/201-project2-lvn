#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>
using namespace llvm;
using namespace std;

#define DEBUG_TYPE "ValueNumbering"

using namespace llvm;

namespace {
		struct ValueNumbering : public FunctionPass {


				static char ID;
				map<Value*, int> varToVersion;              // vincent store variable corresponding with latest version number
				map<pair<Value*, int>, int> valueMap;       // vincent store variable corresponding with version number and value number
				map<int, pair<Value*,int>> valueMap_re;     // vincent store the value number corresponding with variable and its version number
				map<string, int> expressionMap;
				int counter = 1;
				ValueNumbering() : FunctionPass(ID) {}


				bool findInMap(map<pair<Value*, int>, int> valueMap, Value * v){
						for(auto it = valueMap.begin(); it != valueMap.end();++ it){
								if(it->first.first == v) return true;
						}
						return false;
				}

				void findRedundancy(string expr,Value* ptr, string exprOld){
						if(expressionMap.find(expr) == expressionMap.end()) {
								expressionMap[expr] = counter;
								if(!findInMap(valueMap, ptr)){
										valueMap.insert(make_pair(make_pair(ptr, 0),counter));
										valueMap_re[counter] = make_pair(ptr, 0);
								}
								else{
										varToVersion[ptr] += 1;
										valueMap.insert(make_pair(make_pair(ptr, varToVersion[ptr]),counter));
										valueMap_re[counter] = make_pair(ptr, varToVersion[ptr]);
								}
								++ counter;
						}else{

								int tmp_counter = expressionMap[expr];

								errs() << "The redundant computation is: " << *ptr << "\n";
								errs() << "The real computation is:" << exprOld <<"\n";
								if(!findInMap(valueMap, ptr)){
										valueMap.insert(make_pair(make_pair(ptr, 0),tmp_counter));
										valueMap_re[tmp_counter] = make_pair(ptr, 0);
								}
								else{
										varToVersion[ptr] += 1;
										valueMap.insert(make_pair(make_pair(ptr, varToVersion[ptr]),tmp_counter));
										valueMap_re[tmp_counter] = make_pair(ptr, varToVersion[ptr]);
								}

						}
				}
				//vincent create file for output
				ofstream createOutput(Function &F){
						string file = F.getParent() ->getSourceFileName();
						int index = file.find(".");
						file =file.substr(0,index);
						string c = ".out";
						file.append(c);
						ofstream pathOut;
						pathOut.open(file, std::ios::out | std::ios::app);
						return pathOut;
				}
				bool runOnFunction(Function &F) override {
						//create file for output
						ofstream pathOut = createOutput(F);
						if (!pathOut.is_open()) 
								return 0;

						errs() << "ValueNumbering: ";
						errs() << F.getName() << "\n";

						for (auto& basic_block : F){

								//vincent
								errs() << basic_block << "\n";

								for (auto& inst : basic_block){
										if (inst.isBinaryOp()){

												Value* v1 = inst.getOperand(0); // left  oprand
												Value* v2 = inst.getOperand(1); // right oprand
												int vn1;
												int vn2;

												if(!findInMap(valueMap, v1)){   //vincent if not in hashtable
														valueMap.insert(make_pair(make_pair(v1, 0),counter));
														valueMap_re[counter] = make_pair(v1, 0);   //vincent assign a new VN to the variable
														varToVersion[v1] = 0;
														vn1 = counter;
														counter ++;
												}else{
														vn1 = valueMap.at(make_pair(v1, varToVersion[v1]));
												}

												if(!findInMap(valueMap, v2)){
														valueMap.insert(make_pair(make_pair(v2, 0),counter));
														valueMap_re[counter] = make_pair(v2, 0);
														varToVersion[v2] = 0;
														vn2 = counter;
														counter ++;
												}
												else{
														vn2 = valueMap.at(make_pair(v2, varToVersion[v2]));
												}

												string op;
												if(inst.getOpcode() == Instruction::Add){
														op = "+";
												} 
												if(inst.getOpcode() == Instruction::Sub){
														op = "-";
												}
												if(inst.getOpcode() == Instruction::Mul){
														op = "*";
												}
												if(inst.getOpcode() == Instruction::SDiv){
														op = "/";
												}
												//vincent store the value that is not exchanged so that keep output correctly.
												int vn1_old = vn1;
												int vn2_old =vn2;
												if(inst.isCommutative()) {
														if(vn1 > vn2) swap(vn1, vn2);
												}


												string expr = to_string(vn1) + op + to_string(vn2);
												string exprOld = to_string(vn1_old)+op+to_string(vn2_old);

												Value* ptr = dyn_cast<Value>(&inst);
												if(varToVersion.find(ptr) == varToVersion.end()){
														varToVersion.insert(make_pair(ptr, 0));
												}
												//vincent findRedundancy
												findRedundancy(expr, ptr,exprOld);
												pathOut<<expressionMap[expr]<< "=" <<exprOld<<"\n";

										}

								}
						}
						pathOut.close();
						return false;
				}
		}; 
} 

char ValueNumbering::ID = 0;
static RegisterPass<ValueNumbering> X("ValueNumbering", "ValueNumbering Pass",
				false /* Only looks at CFG */,
				false /* Analysis Pass */);
