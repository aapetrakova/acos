#include "falloc.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>

void falloc_init(file_allocator_t* allocator, const char* filepath,
                 uint64_t allowed_page_count) {
  if (allowed_page_count > PAGE_MASK_SIZE * 64) { return; }

  struct stat file_stat;
  _Bool file_exists = 0;
  if (stat(filepath, &file_stat) != -1) {
    allocator->fd = open(filepath, O_RDWR);
    file_exists = 1;
  } else {
    if (errno == ENOENT) {
      allocator->fd = open(filepath, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    } else {
      return;
    }
  }

  if (allocator->fd == -1) { return; }

  if (ftruncate(allocator->fd, ((PAGE_MASK_SIZE + 0xF) >> 3) + PAGE_SIZE * allowed_page_count) == -1) {
    close(allocator->fd);
    return;
  }

  void* page_mask = mmap(NULL, PAGE_SIZE * allowed_page_count + PAGE_MASK_SIZE,
         PROT_READ | PROT_WRITE, MAP_SHARED, allocator->fd, 0);

  if (page_mask == MAP_FAILED) {
    close(allocator->fd);
    return;
  }
  
  allocator->page_mask = page_mask;
  allocator->base_addr = page_mask + PAGE_MASK_SIZE;
  allocator->curr_page_count = 0;
  allocator->allowed_page_count = allowed_page_count;

  if (!file_exists) {
    memset(page_mask, 0, PAGE_MASK_SIZE);
  } else {
    for (size_t page = 0; page < allocator->allowed_page_count; ++page) {
      if ((allocator->page_mask[page >> 6] & (1ULL << (page & 0x3F))) != 0) {
        (allocator->curr_page_count)++;
      }
    }
  }
}

void falloc_destroy(file_allocator_t* allocator) {
  if (allocator->base_addr != NULL) {
    munmap(allocator->base_addr, PAGE_SIZE * allocator->allowed_page_count);
  }
  if (allocator->page_mask != NULL) {
    munmap(allocator->page_mask, PAGE_SIZE * allocator->allowed_page_count + PAGE_MASK_SIZE);
  }
  if (allocator->fd != -1) { close(allocator->fd); }

  allocator->base_addr = NULL;
  allocator->page_mask = NULL;
}

void* falloc_acquire_page(file_allocator_t* allocator) {
  for (uint64_t i = 0; i < allocator->allowed_page_count; i++) {
    uint64_t bit_index = i / 64;
    uint64_t bit_position = i % 64;

    if (!(allocator->page_mask[bit_index] & (1ULL << bit_position))) {
      allocator->page_mask[bit_index] |= (1ULL << bit_position);
      allocator->curr_page_count++;
      return (void*)((char*)allocator->base_addr + (i * PAGE_SIZE));
    }
  }

  return NULL;
}

void falloc_release_page(file_allocator_t* allocator, void** addr) {
  if (addr == NULL) { return; }

  char* base_address = (char*)allocator->base_addr;
  size_t offset = (char*)(*addr) - base_address;
  uint64_t page_index = offset / PAGE_SIZE;

  uint64_t bit_index = page_index / 64;
  uint64_t bit_position = page_index % 64;

  allocator->page_mask[bit_index] &= ~(1ULL << bit_position);
  allocator->curr_page_count--;

  *addr = NULL;
}

