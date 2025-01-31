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

zend_class_entry *mapscript_ce_rect;
#if PHP_VERSION_ID >= 70000
zend_object_handlers mapscript_rect_object_handlers;
#endif  

ZEND_BEGIN_ARG_INFO_EX(rect___get_args, 0, 0, 1)
ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(rect___set_args, 0, 0, 2)
ZEND_ARG_INFO(0, property)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(rect_draw_args, 0, 0, 5)
ZEND_ARG_OBJ_INFO(0, map, mapObj, 0)
ZEND_ARG_OBJ_INFO(0, layer, layerObj, 0)
ZEND_ARG_OBJ_INFO(0, image, imageObj, 0)
ZEND_ARG_INFO(0, classIndex)
ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(rect_project_args, 0, 0, 2)
ZEND_ARG_OBJ_INFO(0, projIn, projectionObj, 0)
ZEND_ARG_OBJ_INFO(0, projOut, projectionObj, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(rect_setExtent_args, 0, 0, 4)
ZEND_ARG_INFO(0, minx)
ZEND_ARG_INFO(0, miny)
ZEND_ARG_INFO(0, maxx)
ZEND_ARG_INFO(0, maxy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(rect_fit_args, 0, 0, 2)
ZEND_ARG_INFO(0, width)
ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

/* {{{ proto rect __construct()
   Create a new rectObj instance. */
PHP_METHOD(rectObj, __construct)
{
  php_rect_object *php_rect;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters_none() == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, getThis());

  if ((php_rect->rect = rectObj_new()) == NULL) {
    mapscript_throw_exception("Unable to construct rectObj." TSRMLS_CC);
    return;
  }
}
/* }}} */

PHP_METHOD(rectObj, __get)
{
  char *property;
  long property_len = 0;
  zval *zobj = getThis();
  php_rect_object *php_rect;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                            &property, &property_len) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, zobj);

  IF_GET_DOUBLE("minx", php_rect->rect->minx)
  else IF_GET_DOUBLE("miny", php_rect->rect->miny)
    else IF_GET_DOUBLE("maxx", php_rect->rect->maxx)
      else IF_GET_DOUBLE("maxy", php_rect->rect->maxy)
        else {
          mapscript_throw_exception("Property '%s' does not exist in this object." TSRMLS_CC, property);
        }
}

/* {{{ proto int draw(mapObj map, layerObj layer, imageObj img, string class_name, string text)
   Draws the individual rect using layer. Returns MS_SUCCESS/MS_FAILURE. */
PHP_METHOD(rectObj, draw)
{
  zval *zobj =  getThis();
  zval *zmap, *zlayer, *zimage;
  char *text = NULL;
  long text_len = 0;
  long classIndex;
  int status = MS_FAILURE;
  php_rect_object *php_rect;
  php_map_object *php_map;
  php_layer_object *php_layer;
  php_image_object *php_image;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOOl|s",
                            &zmap, mapscript_ce_map,
                            &zlayer, mapscript_ce_layer,
                            &zimage, mapscript_ce_image,
                            &classIndex, &text, &text_len) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, zobj);
  php_map = MAPSCRIPT_OBJ_P(php_map_object, zmap);
  php_layer = MAPSCRIPT_OBJ_P(php_layer_object, zlayer);
  php_image = MAPSCRIPT_OBJ_P(php_image_object, zimage);

  if ((status = rectObj_draw(php_rect->rect, php_map->map, php_layer->layer, php_image->image,
                             classIndex, text)) != MS_SUCCESS) {
    mapscript_throw_mapserver_exception("" TSRMLS_CC);
    return;
  }

  RETURN_LONG(status);
}
/* }}} */

PHP_METHOD(rectObj, __set)
{
  char *property;
  long property_len = 0;
  zval *value;
  zval *zobj = getThis();
  php_rect_object *php_rect;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz",
                            &property, &property_len, &value) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, zobj);

  IF_SET_DOUBLE("minx", php_rect->rect->minx, value)
  else IF_SET_DOUBLE("miny", php_rect->rect->miny, value)
    else IF_SET_DOUBLE("maxx", php_rect->rect->maxx, value)
      else IF_SET_DOUBLE("maxy", php_rect->rect->maxy, value)
        else {
          mapscript_throw_exception("Property '%s' does not exist in this object." TSRMLS_CC, property);
        }
}

/* {{{ proto int rect.project(projectionObj in, projectionObj out)
   Project a Rect object Returns MS_SUCCESS/MS_FAILURE */
PHP_METHOD(rectObj, project)
{
  zval *zobj_proj_in, *zobj_proj_out;
  zval *zobj =  getThis();
  php_rect_object *php_rect;
  php_projection_object *php_proj_in, *php_proj_out;
  int status = MS_FAILURE;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO",
                            &zobj_proj_in, mapscript_ce_projection,
                            &zobj_proj_out, mapscript_ce_projection) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, zobj);
  php_proj_in = MAPSCRIPT_OBJ_P(php_projection_object, zobj_proj_in);
  php_proj_out = MAPSCRIPT_OBJ_P(php_projection_object, zobj_proj_out);

  status = rectObj_project(php_rect->rect, php_proj_in->projection, php_proj_out->projection);
  if (status != MS_SUCCESS) {
    mapscript_report_mapserver_error(E_WARNING TSRMLS_CC);
  }

  RETURN_LONG(status);
}
/* }}} */

/* {{{ proto int rect.setextent(xmin, ymin, xmax, ymax)
   Set object property using four values. */
PHP_METHOD(rectObj, setExtent)
{
  zval *zobj =  getThis();
  double minx, miny, maxx, maxy;
  php_rect_object *php_rect;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dddd",
                            &minx, &miny, &maxx, &maxy) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, zobj);

  php_rect->rect->minx = minx;
  php_rect->rect->miny = miny;
  php_rect->rect->maxx = maxx;
  php_rect->rect->maxy = maxy;

  RETURN_LONG(MS_SUCCESS);
}
/* }}} */

/* {{{ proto int rect.fit(int nWidth, int nHeight)
   Adjust extents of the rectangle to fit the width/height specified. */
PHP_METHOD(rectObj, fit)
{
  zval *zobj =  getThis();
  long width, height;
  double retval=0.0;
  php_rect_object *php_rect;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll",
                            &width, &height) == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, zobj);

  retval = rectObj_fit(php_rect->rect, width, height);

  RETURN_DOUBLE(retval);
}
/* }}} */

/* {{{ proto rect getCenter()
   Get center point of the extent. */
PHP_METHOD(rectObj, getCenter)
{
  php_rect_object *php_rect;
  pointObj *center;
  parent_object parent;

  PHP_MAPSCRIPT_ERROR_HANDLING(TRUE);
  if (zend_parse_parameters_none() == FAILURE) {
    PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);
    return;
  }
  PHP_MAPSCRIPT_RESTORE_ERRORS(TRUE);

  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, getThis());

  center = (pointObj *)calloc(1, sizeof(pointObj));
  if (!center) {
    mapscript_report_mapserver_error(E_WARNING TSRMLS_CC);
    RETURN_NULL();
  }

  center->x = (php_rect->rect->minx + php_rect->rect->maxx)/2;
  center->y = (php_rect->rect->miny + php_rect->rect->maxy)/2;

  /* Return result object */
  MAPSCRIPT_MAKE_PARENT(NULL, NULL);
  mapscript_create_point(center, parent, return_value TSRMLS_CC);
}
/* }}} */

zend_function_entry rect_functions[] = {
  PHP_ME(rectObj, __construct, no_args, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  PHP_ME(rectObj, getCenter, no_args, ZEND_ACC_PUBLIC)
  PHP_ME(rectObj, __get, rect___get_args, ZEND_ACC_PUBLIC)
  PHP_ME(rectObj, __set, rect___set_args, ZEND_ACC_PUBLIC)
  PHP_MALIAS(rectObj, set, __set, rect___set_args, ZEND_ACC_PUBLIC)
  PHP_ME(rectObj, draw, rect_draw_args, ZEND_ACC_PUBLIC)
  PHP_ME(rectObj, project, rect_project_args, ZEND_ACC_PUBLIC)
  PHP_ME(rectObj, setExtent, rect_setExtent_args, ZEND_ACC_PUBLIC)
  PHP_ME(rectObj, fit, rect_fit_args, ZEND_ACC_PUBLIC) {
    NULL, NULL, NULL
  }
};


void mapscript_create_rect(rectObj *rect, parent_object parent, zval *return_value TSRMLS_DC)
{
  php_rect_object * php_rect;
  object_init_ex(return_value, mapscript_ce_rect);
  php_rect = MAPSCRIPT_OBJ_P(php_rect_object, return_value);
  php_rect->rect = rect;

  if(ZVAL_NOT_UNDEF(parent.val))
    php_rect->is_ref = 1;

  php_rect->parent = parent;
  MAPSCRIPT_ADDREF(parent.val);
}

#if PHP_VERSION_ID >= 70000
/* PHP7 - Modification by Bjoern Boldt <mapscript@pixaweb.net> */
static zend_object *mapscript_rect_create_object(zend_class_entry *ce TSRMLS_DC)
{
  php_rect_object *php_rect;

  php_rect = ecalloc(1, sizeof(*php_rect) + zend_object_properties_size(ce));

  zend_object_std_init(&php_rect->zobj, ce TSRMLS_CC);
  object_properties_init(&php_rect->zobj, ce);

  php_rect->zobj.handlers = &mapscript_rect_object_handlers;

  MAPSCRIPT_INIT_PARENT(php_rect->parent);
  php_rect->is_ref = 0;

  return &php_rect->zobj;
}

static void mapscript_rect_free_object(zend_object *object)
{
  php_rect_object *php_rect;

  php_rect = (php_rect_object *)((char *)object - XtOffsetOf(php_rect_object, zobj));

  MAPSCRIPT_FREE_PARENT(php_rect->parent);

  if (php_rect->rect && !php_rect->is_ref) {
    rectObj_destroy(php_rect->rect);
  }

  zend_object_std_dtor(object);
}

//PHP8
#if PHP_VERSION_ID >= 80000
static zend_object* mapscript_rect_clone_object(zend_object *zobj_old)
{
  php_rect_object *php_rect_old, *php_rect_new;
  zend_object* zobj_new;

  php_rect_old = MAPSCRIPT_OBJ_Z(php_rect_object, zobj_old);

  zobj_new = mapscript_rect_create_object(mapscript_ce_rect);
  php_rect_new = MAPSCRIPT_OBJ_Z(php_rect_object, zobj_new);

  zend_objects_clone_members(&php_rect_new->zobj, &php_rect_old->zobj);

  if ((php_rect_new->rect = rectObj_new()) == NULL) {
    mapscript_throw_exception("Unable to construct rectObj." TSRMLS_CC);
    return NULL;
  }
  memcpy(php_rect_new->rect, php_rect_old->rect, sizeof(rectObj));

  return zobj_new;
}
#else
static zend_object* mapscript_rect_clone_object(zval *zobj)
{
  php_rect_object *php_rect_old, *php_rect_new;
  zend_object* zobj_new;

  php_rect_old = MAPSCRIPT_OBJ_P(php_rect_object, zobj);

  zobj_new = mapscript_rect_create_object(mapscript_ce_rect);
  php_rect_new = (php_rect_object *)((char *)zobj_new - XtOffsetOf(php_rect_object, zobj));

  zend_objects_clone_members(&php_rect_new->zobj, &php_rect_old->zobj);

  if ((php_rect_new->rect = rectObj_new()) == NULL) {
    mapscript_throw_exception("Unable to construct rectObj." TSRMLS_CC);
    return NULL;
  }
  memcpy(php_rect_new->rect, php_rect_old->rect, sizeof(rectObj));

  return zobj_new;
}
#endif

PHP_MINIT_FUNCTION(rect)
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, "rectObj", rect_functions);
  mapscript_ce_rect = zend_register_internal_class(&ce TSRMLS_CC);

  mapscript_ce_rect->create_object = mapscript_rect_create_object;
  mapscript_ce_rect->ce_flags |= ZEND_ACC_FINAL;

  memcpy(&mapscript_rect_object_handlers, &mapscript_std_object_handlers, sizeof(mapscript_rect_object_handlers));
  mapscript_rect_object_handlers.free_obj = mapscript_rect_free_object;
  mapscript_rect_object_handlers.clone_obj = mapscript_rect_clone_object;
  mapscript_rect_object_handlers.offset   = XtOffsetOf(php_rect_object, zobj);

  return SUCCESS;
}
#else
/* PHP5 */
static void mapscript_rect_object_destroy(void *object TSRMLS_DC)
{
  php_rect_object *php_rect = (php_rect_object *)object;

  MAPSCRIPT_FREE_OBJECT(php_rect);

  MAPSCRIPT_FREE_PARENT(php_rect->parent);

  if (php_rect->rect && !php_rect->is_ref) {
    rectObj_destroy(php_rect->rect);
  }

  efree(object);
}

static zend_object_value mapscript_rect_object_new(zend_class_entry *ce TSRMLS_DC)
{
  zend_object_value retval;
  php_rect_object *php_rect;

  MAPSCRIPT_ALLOC_OBJECT(php_rect, php_rect_object);

  retval = mapscript_object_new(&php_rect->std, ce,
                                &mapscript_rect_object_destroy TSRMLS_CC);

  php_rect->is_ref = 0;
  MAPSCRIPT_INIT_PARENT(php_rect->parent)

  return retval;
}

PHP_MINIT_FUNCTION(rect)
{
  zend_class_entry ce;

  MAPSCRIPT_REGISTER_CLASS("rectObj",
                           rect_functions,
                           mapscript_ce_rect,
                           mapscript_rect_object_new);

  mapscript_ce_rect->ce_flags |= ZEND_ACC_FINAL_CLASS;

  return SUCCESS;
}
#endif
