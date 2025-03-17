#ifndef __COMMON_H
#define __COMMON_H

/* POSIX required definition */
#define _XOPEN_SOURCE 700

#include <core/apidef.h>
#include <nappgui.h>

/**
 * @brief   Enumeration of C types, sorted by size
 * 
 */
typedef enum 
{
    /* trivial */
    CTYPE_NULL,                 
    CTYPE_BOOL,                 
    CTYPE_CHAR,                 
    CTYPE_UNSIGNED_CHAR,        
    CTYPE_SIGNED_CHAR,          
    CTYPE_CONST_CHAR,           
    CTYPE_SHORT,                
    CTYPE_UNSIGNED_SHORT,       
    CTYPE_INT,                  
    CTYPE_UNSIGNED_INT,         
    CTYPE_LONG,                 
    CTYPE_UNSIGNED_LONG,        
    CTYPE_FLOAT,                
    CTYPE_LONG_LONG,            
    CTYPE_UNSIGNED_LONG_LONG,   
    CTYPE_VOID,                 
    CTYPE_DOUBLE,               
    CTYPE_LONG_DOUBLE,          
    /* non-trivial */
    CTYPE_UINT_VEC_2D,          
    CTYPE_INT_VEC_2D,           
    CTYPE_UINT8_VEC_2D,         
    CTYPE_INT8_VEC_2D,          
    CTYPE_UINT16_VEC_2D,        
    CTYPE_INT16_VEC_2D,         
    CTYPE_UINT32_VEC_2D,        
    CTYPE_INT32_VEC_2D,         
    CTYPE_FLOAT_VEC_2D,         
    CTYPE_DOUBLE_VEC_2D,        
    /*  */
    CTYPE_MAX
} ECType;

typedef struct _app_t App;

struct _app_t
{
    int argc;
    char** argv;
    Window *pWindow;
    TextView *pText;
    ImageView *pImageView;
    Layout *pLayout;
};

typedef enum
{
    CPANEL_NULL,
    CPANEL_AUTOCONFIG,
    CPANEL_MAIN,
    CPANEL_MAX
} EPanelType;

#define UNUSED(x) (void)x
#define IS_VALID(ptr) ((ptr) != 0x0 && (ptr) != NULL) /**< General check for pointers */

#endif
