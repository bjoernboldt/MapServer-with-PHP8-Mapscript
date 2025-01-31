/**********************************************************************
 * $Id: php_mapscript.c 9765 2010-01-28 15:32:10Z aboudreault $
 *
 * Project:  MapServer
 * Purpose:  PHP/MapScript extension for MapServer.  External interface
 *           functions
 * Author:   Daniel Morissette, DM Solutions Group (dmorissette@dmsolutions.ca)
 *           Alan Boudreault, Mapgears
 *
 **********************************************************************
 * Copyright (c) 2000-2010, Daniel Morissette, DM Solutions Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of this Software or works derived from this Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "php_mapscript.h"

#if PHP_VERSION_ID >= 50625 && PHP_VERSION_ID < 70000
#undef ZVAL_STRING
#define ZVAL_STRING(z, s, duplicate) do {       \
    const char *__s=(s);                            \
    zval *__z = (z);                                        \
    Z_STRLEN_P(__z) = strlen(__s);          \
    Z_STRVAL_P(__z) = (duplicate?estrndup(__s, Z_STRLEN_P(__z)):(char*)__s);\
    Z_TYPE_P(__z) = IS_STRING;                      \
} while (0)
#endif

zend_class_entry *mapscript_ce_error;
#if PHP_VERSION_ID >= 70000
zend_object_handlers mapscript_error_object_handlers;
#endif  

ZEND_BEGIN_ARG_INFO_EX(error___get_args, 0, 0, 1)
ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(error___set_args, 0, 0, 2)
ZEND_ARG_INFO(0, property)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

/* {{{ proto error __construct()
   errorObj CANNOT be instanciated, this will throw an exception on use */
PHP_METHOD(errorObj, __construct)
{
  mapscript_throw_exception("errorObj cannot be constructed" TSRMLS_CC);
}
/* }}} */

PHP_METHOD(errorObj, __get)
{
  char *property;
  long property_len = 0;
  zval *zobj = getThis();
  /* php_error is in PHP7 defined in php.h, so we use php_errobj instead */
  php_error_object *php_errobj;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                            &property, &property_len) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_errobj = MAPSCRIPT_OBJ_P(php_error_object, zobj);

  IF_GET_LONG("code", php_errobj->error->code)
  else IF_GET_STRING("routine", php_errobj->error->routine)
    else IF_GET_STRING("message", php_errobj->error->message)
      else IF_GET_LONG("isreported", php_errobj->error->isreported)
        else {
          mapscript_throw_exception("Property '%s' does not exist in this object." TSRMLS_CC, property);
        }
}

PHP_METHOD(errorObj, __set)
{
  char *property;
  long property_len = 0;
  zval *value;
  /* zval *zobj = getThis(); */
  /* php_error_object *php_errobj; */

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz",
                            &property, &property_len, &value) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  /* php_errobj = MAPSCRIPT_OBJ_P(php_error_object, zobj); */

  if ( (STRING_EQUAL("code", property)) ||
       (STRING_EQUAL("routine", property)) ||
       (STRING_EQUAL("isreported", property)) ||
       (STRING_EQUAL("message", property))) {
    mapscript_throw_exception("Property '%s' is read-only and cannot be set." TSRMLS_CC, property);
  } else {
    mapscript_throw_exception("Property '%s' does not exist in this object." TSRMLS_CC, property);
  }
}

/* {{{ proto int error.next()
   Returns a ref to the next errorObj in the list, or NULL if we reached the last one */
PHP_METHOD(errorObj, next)
{
  zval *zobj = getThis();
  php_error_object *php_errobj;
  errorObj *error = NULL;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters_none() == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_errobj = MAPSCRIPT_OBJ_P(php_error_object, zobj);

  if (php_errobj->error->next == NULL)
    RETURN_NULL();

  /* Make sure 'self' is still valid.  It may have been deleted by
   * msResetErrorList() */
  error = msGetErrorObj();
  while(error != php_errobj->error) {
    if (error->next == NULL) {
      mapscript_throw_exception("Trying to access an errorObj that has expired." TSRMLS_CC);
      return;
    }
    error = error->next;
  }

  php_errobj->error = php_errobj->error->next;
  *return_value = *zobj;
  zval_copy_ctor(return_value);
  INIT_PZVAL(return_value);
}
/* }}} */

zend_function_entry error_functions[] = {
  PHP_ME(errorObj, __construct, no_args, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  PHP_ME(errorObj, __get, error___get_args, ZEND_ACC_PUBLIC)
  PHP_ME(errorObj, __set, error___set_args, ZEND_ACC_PUBLIC)
  PHP_ME(errorObj, next, no_args, ZEND_ACC_PUBLIC) {
    NULL, NULL, NULL
  }
};

void mapscript_create_error(errorObj *error, zval *return_value TSRMLS_DC)
{
  php_error_object * php_errobj;
  object_init_ex(return_value, mapscript_ce_error);
  php_errobj = MAPSCRIPT_OBJ_P(php_error_object, return_value);
  php_errobj->error = error;
}

#if PHP_VERSION_ID >= 70000
/* PHP7 - Modification by Bjoern Boldt <mapscript@pixaweb.net> */
static zend_object *mapscript_error_create_object(zend_class_entry *ce TSRMLS_DC)
{
  php_error_object *php_errobj;

  php_errobj = ecalloc(1, sizeof(*php_errobj) + zend_object_properties_size(ce));

  zend_object_std_init(&php_errobj->zobj, ce TSRMLS_CC);
  object_properties_init(&php_errobj->zobj, ce);

  php_errobj->zobj.handlers = &mapscript_error_object_handlers;

  return &php_errobj->zobj;
}

/*
static void mapscript_error_free_object(zend_object *object)
{
  php_error_object *php_errobj;

  php_errobj = (php_error_object *)((char *)object - XtOffsetOf(php_error_object, zobj));

  zend_object_std_dtor(object);
}
*/

PHP_MINIT_FUNCTION(error)
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, "errorObj", error_functions);
  mapscript_ce_error = zend_register_internal_class(&ce TSRMLS_CC);

  mapscript_ce_error->create_object = mapscript_error_create_object;
  mapscript_ce_error->ce_flags |= ZEND_ACC_FINAL;

  memcpy(&mapscript_error_object_handlers, &mapscript_std_object_handlers, sizeof(mapscript_error_object_handlers));
  /* mapscript_error_object_handlers.free_obj = mapscript_error_free_object; // nothing to do here -> use standard handler */
  mapscript_error_object_handlers.offset   = XtOffsetOf(php_error_object, zobj);

  return SUCCESS;
}
#else
/* PHP5 */
static void mapscript_error_object_destroy(void *object TSRMLS_DC)
{
  php_error_object *php_errobj = (php_error_object *)object;

  MAPSCRIPT_FREE_OBJECT(php_errobj);

  /* We don't need to free the errorObj */

  efree(object);
}

static zend_object_value mapscript_error_object_new(zend_class_entry *ce TSRMLS_DC)
{
  zend_object_value retval;
  php_error_object *php_errobj;

  MAPSCRIPT_ALLOC_OBJECT(php_errobj, php_error_object);

  retval = mapscript_object_new(&php_errobj->std, ce,
                                &mapscript_error_object_destroy TSRMLS_CC);

  return retval;
}

PHP_MINIT_FUNCTION(error)
{
  zend_class_entry ce;

  MAPSCRIPT_REGISTER_CLASS("errorObj",
                           error_functions,
                           mapscript_ce_error,
                           mapscript_error_object_new);

  mapscript_ce_error->ce_flags |= ZEND_ACC_FINAL_CLASS;

  return SUCCESS;
}
#endif
