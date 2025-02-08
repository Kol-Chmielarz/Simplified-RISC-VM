#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "literal_table.h"
#include "utilities.h"

// Return the size (in words/entries) in the literal table
typedef struct literal_table_entry_s
{
    struct literal_table_entry_s *next;
    const char *text;
    word_type value;
    unsigned int word_offset;
} literal_table_entry_t;

static literal_table_entry_t *first;
static literal_table_entry_t *last;
static unsigned int next_word_offset;

// Iteration state follows
static bool iterating;
static literal_table_entry_t *iteration_next;

// Return the size (in words/entries) in the literal table
unsigned int literal_table_size()
{
    return next_word_offset;
}

// Return if the table is empty
bool literal_table_empty()
{
    return next_word_offset == 0;
}

// Return if the table is full
bool literal_table_full()
{
    return false;
}

// initialize the literal_table
void literal_table_initialize()
{
    first = NULL;
    last = NULL;
    next_word_offset = 0;
    iterating = false;
    iteration_next = NULL;
}

// Return the offset of sought if it is in the table,
int literal_table_find_offset(const char *sought, word_type value)
{
    literal_table_entry_t *cur = first;

    while (cur != NULL)
    {
        if (strcmp(cur->text, sought) == 0)
        {
            return cur->word_offset;
        }
        cur = cur->next;
    }
    return -1;
}

// Return true just when sought is in the table
bool literal_table_present(const char *sought, word_type value)
{
    return literal_table_find_offset(sought, value) != -1;
}

// Return the word offset for val_string/value
unsigned int literal_table_lookup(const char *val_string,
                                  word_type value)
{
    int ret = literal_table_find_offset(val_string, value);

    if (ret >= 0)
    {
        return ret;
    }
    literal_table_entry_t *new = malloc(sizeof(literal_table_entry_t));

    new->text = val_string;
    new->value = value;
    new->next = NULL;
    ret = next_word_offset;
    new->word_offset = next_word_offset++;
    if (new == NULL)
    {
        bail_with_error("No space for allocate ");
    }
    if (first == NULL)
    {
        first = new;
        last = new;
    }
    else
    {
        last->next = new;
        last = new;
    }

    return ret;
}

// Start an iteration over the literal table
void literal_table_start_iteration()
{
    if (iterating)
    {
        bail_with_error("Already iterating");
    }
    iterating = true;
    iteration_next = first;
}

// End the current iteration over the literal table.
void literal_table_end_iteration()
{
    iterating = false;
}

// Is there another literal in the literal table?
bool literal_table_iteration_has_next()
{
    bool ret = (iteration_next != NULL);
    if (!ret)
    {
        iterating = false;
    }
    return ret;
}

// Return the next word_type in the literal table
word_type literal_table_iteration_next()
{
    assert(iteration_next != NULL);
    float ret = iteration_next->value;
    iteration_next = iteration_next->next;
    return ret;
}
