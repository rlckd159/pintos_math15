#include "vm/frame.h"
#include "threads/palloc.h"
void frame_init(void){
    lock_init(&frame_lock);
    hash_init(&frame_table, frame_val, frame_less, NULL);
}

int frame_val(struct hash_elem* hash, void* aux){
    struct frame_entry* frame = hash_entry(hash, struct frame_entry, frame_elem);
    return frame->kaddr;
}

bool frame_less(struct hash_elem* a, struct hash_elem* b, void* aux){
    struct frame_entry* frame_a = hash_entry(a, struct frame_entry, frame_elem);
    struct frame_entry* frame_b = hash_entry(b, struct frame_entry, frame_elem);
    return frame_a->kaddr < frame_b->kaddr;
}

bool frame_insert(void* uaddr, void* kaddr){
    struct frame_entry* frame = malloc(sizeof(struct frame_entry));
    frame->kaddr = kaddr;
    frame->uaddr = uaddr;
    if(hash_insert(&frame_table, &frame->frame_elem) == NULL)
        return false;
    else
    {
        return true;
    }
}

void* frame_get_page(enum palloc_flags flags, void* uaddr){
    if(!(flags & PAL_USER))
        return NULL;
    
    void* frame = palloc_get_page(flags);
  
    /* free-page left*/
    if(frame!= NULL){
        frame_insert(uaddr, frame);
        return frame;
    }
    /* we need eviction */
    else{
        if(frame_table_clock == NULL)
            hash_first(frame_table_clock, &frame_table);
        while(frame_table_clock != NULL){
            frame = hash_entry(hash_cur(frame_table_clock), struct frame_entry, frame_elem);
            hash_next(frame_table_clock);
        } 
        printf("sex\n");
        return NULL;
    }
    return NULL;
}

void frame_free_page(void* frame){
    struct frame_entry* real_frame = frame;
    struct hash_elem* e = hash_delete(&frame_table, &real_frame->frame_elem);
    if(e!= NULL)
        free(hash_entry(e, struct frame_entry, frame_elem));
    palloc_free_page(real_frame->kaddr);
}

void frame_destroy(void* frame_table){
    
}
