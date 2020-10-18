#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "userprog/pagedir.h"
static void syscall_handler (struct intr_frame *);
struct lock filesys_lock;

struct one_file{
  struct file* file;
  struct list_elem file_elem;
  int fd;
};



void
syscall_init (void) 
{
  
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int args[10];
  
  switch(*(int*)f->esp){
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      get_args(f->esp+4, &args[0], 1);
      exit(args[0]);
      
      break;
    case SYS_EXEC: 
      get_args(f->esp+4, &args[0], 1);
      f->eax = exec((const char*)args[0]);
      break;
    case SYS_WAIT:
      get_args(f->esp+4, &args[0], 1);
      
      f->eax = wait((int)args[0]);
      break;
    case SYS_CREATE:
      get_args(f->esp+4, &args[0], 2);
      f->eax = create((const char*)args[0], (unsigned)args[1]);
      break;
    case SYS_REMOVE:
      get_args(f->esp+4, &args[0], 1);
      f->eax = remove((const char*)args[0]);
      break;
    case SYS_OPEN:
      get_args(f->esp+4, &args[0], 1);
      f->eax = open((const char*)args[0]);
      break;
    case SYS_FILESIZE:
      break;
    case SYS_READ:
      break;
    case SYS_WRITE:
      get_args(f->esp+4, &args[0], 3);
      f->eax = write((int)args[0],(const void*)args[1], (unsigned int)args[2]);
      break;
    case SYS_SEEK:
      break;
    case SYS_TELL:
      break;
    case SYS_CLOSE:
      break;
    default:
      break;              
  }
  
  //printf ("system call! %d\n", *(int*)f->esp);
  //thread_exit ();
}

void halt(){
  shutdown_power_off();
}

void exit(int status){
 
  printf("%s: exit(%d)\n", thread_current()->name,status);
   thread_current()->exit_status = status; 
  //printf("%s\n",((char *) 0x20101234));
  thread_exit();
}

bool create(const char* file, unsigned initial_size){
  if(file == NULL){
    exit(-1);
  }
  if(!file_available(file))
    exit(-1);
  lock_acquire(&filesys_lock);
  bool file_creation = filesys_create(file, initial_size);
  lock_release(&filesys_lock);
  return file_creation;
}

bool remove(const char* file){
  return filesys_remove(file);
}

tid_t exec(const char* cmd_line){
  lock_acquire(&filesys_lock);
  tid_t t = process_execute(cmd_line);
  lock_release(&filesys_lock);
  return t;
}

int wait(tid_t pid){
  return process_wait(pid);
}

int write(int fd, const void* buffer, unsigned size){ 
 if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  return -1;
}

int open(const char* file){
  if(!file_available(file))
    return -1;

  lock_acquire(&filesys_lock);
  struct file* f = filesys_open(file);
  lock_release(&filesys_lock);
  
  if (f ==NULL)
    return -1; 
  else
  {
    struct one_file* new_file = malloc(sizeof(struct one_file));
    new_file->fd = new_fid();
    new_file->file = f;
    list_push_back(&thread_current()->files_list, &new_file->file_elem);
    return new_file->fd;
  } 
 
  
}


void available_addr(void* addr){
  if(!is_user_vaddr(addr)){
    exit(-1);
    return;
  }
}

void get_args(void* esp, int* arg, int count){
  int* temp;
  for(int i = 0; i < count; i++){
    temp = (int*)esp + i;
    available_addr((void*)temp);
    arg[i] = *temp;
  }
}

int file_available(void* addr){
  if (addr > PHYS_BASE)
    return 0;
  if (addr < 0x08048000)
    return 0;
  if (pagedir_get_page(thread_current()->pagedir, addr) == NULL)
    return 0;
  return 1;
}

int new_fid(void){
  static int fid = 2;
  return fid++;
}