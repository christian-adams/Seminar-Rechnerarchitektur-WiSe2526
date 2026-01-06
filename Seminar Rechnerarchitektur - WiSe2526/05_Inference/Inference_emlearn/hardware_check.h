#pragma once

int GetFreeSRAM(float* percentage) 
{
  int freeHeap = rp2040.getFreeHeap();
  *percentage = freeHeap * 100 / (float)rp2040.getTotalHeap();
  return freeHeap;
}

int GetUsedSRAM(float* percentage) 
{
  int usedHeap = rp2040.getUsedHeap();
  *percentage = usedHeap * 100 / (float)rp2040.getTotalHeap();
  return usedHeap;
}