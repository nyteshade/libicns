/*
File:       icns_element_list.c
Copyright (C) 2023 Brielle Harrison <nyteshade@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "icns_internals.h"
#include "icns.h"

int icns_create_element_list(icns_element_list_t** elementListOut)
{
  *elementListOut = calloc(1, sizeof(icns_element_list_t));
  if(*elementListOut == NULL)
  {
    icns_print_err("icns_create_element_list: cannot allocate memory for element list\n");
    return ICNS_STATUS_NO_MEMORY;
  }
  return ICNS_STATUS_OK;
}

int icns_copy_element_to_list(icns_element_list_t* list, const icns_element_t* element)
{
  icns_element_list_t* node = NULL;
  icns_element_t* copy = NULL;
  icns_size_t size = ICNS_NULL_SIZE;
  icns_type_t type = ICNS_NULL_TYPE;
  icns_bool_t dataIsBigEndian = kICNSFalse;

  if(list == NULL)
  {
    icns_print_err("icns_copy_element_to_list: no list to copy data to\n");
    return ICNS_STATUS_MISSING_LIST;
  }

  if(element == NULL)
  {
    icns_print_err("icns_copy_element_to_list: no element to copy data from\n");
    return ICNS_STATUS_MISSING_ELEMENT;
  }

  if(!icns_known_type(element->elementType))
  {
    memcpy_be(&(type), &(element->elementType), sizeof(icns_type_t));
    if (!icns_known_type(type))
    {
      char typeStr[5];
      icns_type_str(type, typeStr);
      icns_print_err("icns_copy_element_to_list: unknown element type '%s'\n", typeStr);
      return ICNS_STATUS_INVALID_DATA;
    }

    memcpy_be(&(size), &(element->elementSize), sizeof(icns_size_t));
    dataIsBigEndian = kICNSTrue;
  }

  copy = icns_dup_element(element, size, dataIsBigEndian);
  if(!copy)
  {
    icns_print_err("icns_copy_element_to_list: unable to duplicate element\n");
    return ICNS_STATUS_NO_MEMORY;
  }

  icns_print_element(copy, "icns_copy_element_to_list", 2);

  if(list->element == NULL)
  {
    list->element = copy;
    return ICNS_STATUS_OK;
  }

  for (node = list; node->next; node = node->next);
  if (icns_create_element_list(&node->next) == ICNS_STATUS_OK)
  {
    node->next->element = copy;
    return ICNS_STATUS_OK;
  }
  else 
  {
    icns_print_err("icns_copy_element_to_list: unable to extend supplied list\n");
  }

  return ICNS_STATUS_NO_MEMORY;
}

int icns_add_element_to_list(icns_element_list_t* list, icns_element_t* element)
{
  icns_element_list_t* node = NULL;

  if (list == NULL)
  {
    icns_print_err("icns_add_element_to_list: no list to copy data to\n");
    return ICNS_STATUS_MISSING_LIST;
  }

  if (element == NULL)
  {
    icns_print_err("icns_add_element_to_list: no element to copy data from\n");
    return ICNS_STATUS_MISSING_ELEMENT;
  }

  if (list->element == NULL)
  {
    list->element = element;
    return ICNS_STATUS_OK;
  }

  for (node = list; node->next; node = node->next);
  if (icns_create_element_list(&node->next) == ICNS_STATUS_OK)
  {
    node->next->element = element;
    return ICNS_STATUS_OK;
  }

  return ICNS_STATUS_NO_MEMORY;
}

int icns_free_element_list(icns_element_list_t* list, icns_bool_t freeElementsAndData)
{
  icns_element_list_t* current = list;
  icns_element_list_t* next;

  if(list == NULL)
  {
    icns_print_err("icns_free_element_list: no list provided to free\n");
    return ICNS_STATUS_MISSING_LIST;
  }

  while (current != NULL) {
    next = current->next;

    if (freeElementsAndData) {
      icns_element_t* element = current->element;
      if (element != NULL) {
        free(element->elementData);
        free(element);
      }
    }

    free(current);
    current = next;
  }

  return ICNS_STATUS_OK;
}
