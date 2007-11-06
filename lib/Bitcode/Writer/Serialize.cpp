//==- Serialize.cpp - Generic Object Serialization to Bitcode ----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by Ted Kremenek and is distributed under the
// University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the internal methods used for object serialization.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/Serialize.h"
#include "string.h"

using namespace llvm;

Serializer::Serializer(BitstreamWriter& stream)
  : Stream(stream), BlockLevel(0) {}

Serializer::~Serializer() {
  if (inRecord())
    EmitRecord();

  while (BlockLevel > 0)
    Stream.ExitBlock();
   
  Stream.FlushToWord();
}

void Serializer::EmitRecord() {
  assert(Record.size() > 0 && "Cannot emit empty record.");
  Stream.EmitRecord(8,Record);
  Record.clear();
}

void Serializer::EnterBlock(unsigned BlockID,unsigned CodeLen) {
  FlushRecord();
  Stream.EnterSubblock(BlockID,CodeLen);
  ++BlockLevel;
}

void Serializer::ExitBlock() {
  assert (BlockLevel > 0);
  --BlockLevel;
  FlushRecord();
  Stream.ExitBlock();
}

void Serializer::EmitInt(unsigned X) {
  assert (BlockLevel > 0);
  Record.push_back(X);
}

void Serializer::EmitCStr(const char* s, const char* end) {
  Record.push_back(end - s);
  
  while(s != end) {
    Record.push_back(*s);
    ++s;
  }

  EmitRecord();
}

void Serializer::EmitCStr(const char* s) {
  EmitCStr(s,s+strlen(s));
}

unsigned Serializer::getPtrId(const void* ptr) {
  if (!ptr)
    return 0;
  
  MapTy::iterator I = PtrMap.find(ptr);
  
  if (I == PtrMap.end()) {
    unsigned id = PtrMap.size()+1;
    PtrMap[ptr] = id;
    return id;
  }
  else return I->second;
}


#define INT_EMIT(TYPE)\
void SerializeTrait<TYPE>::Emit(Serializer&S, TYPE X) { S.EmitInt(X); }

INT_EMIT(bool)
INT_EMIT(unsigned char)
INT_EMIT(unsigned short)
INT_EMIT(unsigned int)
INT_EMIT(unsigned long)
