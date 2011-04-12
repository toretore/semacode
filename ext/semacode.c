/*

  == Introduction
  
  This Ruby extension implements a DataMatrix encoder for Ruby. It is
  typically used to create semacodes, which are barcodes, that contain URLs.
  This encoder does not create image files or visual representations of the
  semacode. This is because it can be used for more than creating images, such
  as rendering semacodes to HTML, SVG, PDF or even stored in a database or
  file for later use.

  Author::    Guido Sohne  (mailto:guido@sohne.net)
  Copyright:: Copyright (c) 2007 Guido Sohne
  License::   GNU GPL version 2, available from http://www.gnu.org
*/

#include "ruby.h"
#include "semacode.h"

/*

Internal function that encodes a string of given length, storing
the encoded result into an internal, private data structure. This
structure is consulted for any operations, such as to get the 
semacode dimensions. It deallocates any previous data before 
generating a new encoding.

Due to a bug in the underlying encoder, we do two things

 * append a space character before encoding, to get around
   an off by one error lurking in the C code
   
 * manually select the best barcode dimensions, to avoid
   an encoder bug where sometimes no suitable encoding would
   be found

*/
semacode_t* 
encode_string(semacode_t *semacode, int message_length, char *message)
{
  /* avoid obvious bad cases */
  if(semacode == NULL || message == NULL || message_length < 1) {
    return NULL;
  }
    
  /* deallocate if exists already */
  if(semacode->data != NULL)
    free(semacode->data);

  /* deallocate if exists already */
  if(semacode->encoding != NULL)
    free(semacode->encoding);
        
  bzero(semacode, sizeof(semacode_t));
  
  // work around encoding bug by appending an extra character.
  strcat(message, " ");
  message_length++;
  
  // choose the best grid that will hold our message
  iec16022init(&semacode->width, &semacode->height, message);
  
  // encode the actual data
  semacode->data = (char *) iec16022ecc200(
    &semacode->width, 
    &semacode->height, 
    &semacode->encoding, 
    message_length, 
    (unsigned char *) message, 
    &semacode->raw_encoded_length,
    &semacode->symbol_capacity, 
    &semacode->ecc_bytes);

  return semacode;
}

/* our module and class, respectively */

static VALUE rb_mSemacode;
static VALUE rb_cEncoder;

static void
semacode_mark(semacode_t *semacode)
{
  /* need this if we ever hold any other Ruby objects. so let's not go there. */
}

static void
semacode_free(semacode_t *semacode)
{
  if(semacode != NULL) {
    if(semacode->data != NULL)
      free(semacode->encoding);
      free(semacode->data);      
    /* zero before freeing */
    bzero(semacode, sizeof(semacode));
    free(semacode);
  }
}

static VALUE
semacode_allocate(VALUE klass)
{
  semacode_t *semacode;
  return Data_Make_Struct(klass, semacode_t, semacode_mark, semacode_free, semacode); 
}

/* 
  Initialize the semacode. This function is called after a semacode is
  created. Ruby objects are created using a new method, and then initialized
  via the 'initialize' method once they have been allocated.
  
  The initializer takes a single argument, which can be anything that 
  responds to the 'to_s' method - that is, anything string like.
  
  The string in the argument is encoded and the semacode is returned
  initialized and ready for use.
  
*/
static VALUE
semacode_init(VALUE self, VALUE message)
{
  semacode_t *semacode;
  
  if (!rb_respond_to(message, rb_intern ("to_s")))
      rb_raise(rb_eRuntimeError, "target must respond to 'to_s'");

  Data_Get_Struct(self, semacode_t, semacode);
  encode_string(semacode, StringValueLen(message), StringValuePtr(message));
  
  return self;
}

/*
  This function turns the raw output from an encoding into a more
  friendly format organized by rows and columns.
  
  It returns the semacode matrix as an array of arrays of boolean. The
  first element in the array is the top row, the last is the bottom row.
  
  Each row is also an array, containing boolean values. The length of each
  row is the same as the semacode width, and the number of rows is the same
  as the semacode height.

*/
static VALUE
semacode_grid(semacode_t *semacode)
{
  int w = semacode->width;
  int h = semacode->height;
  
  VALUE ret = rb_ary_new2(h);

  int x, y;
	for (y = h - 1; y >= 0; y--) {
	  VALUE ary = rb_ary_new2(w);
		for (x = 0; x < w; x++) {
		  if(semacode->data[y * w + x])
		    rb_ary_push(ary, Qtrue);
		  else
		    rb_ary_push(ary, Qfalse);
		}
		rb_ary_push(ret, ary);
	}
	
	return ret;
}

/*
  This function turns the raw output from an encoding into a string
  representation.
  
  It returns the semacode matrix as a comma-separated list of character
  vectors (sequence of characters). The top row is the first vector and
  the bottow row is the last vector.
  
  Each vector is a sequence of characters, either '1' or '0', to represent
  the bits of the semacode pattern. The length of a vector is the semacode
  width, and the number of vectors is the same as the semacode height.

*/
static VALUE
semacode_to_s(VALUE self)
{
  semacode_t *semacode;
  VALUE str;
  int x, y;  
  int w, h;
  
  Data_Get_Struct(self, semacode_t, semacode);
  
  if(semacode == NULL || semacode->data == NULL)
    return Qnil;
  
  w = semacode->width;
  h = semacode->height;
  
  str = rb_str_new2("");
  
	for (y = h - 1; y >= 0; y--) {
		for (x = 0; x < w; x++) {
		  if(semacode->data[y * w + x])
		    rb_str_cat(str, "1", 1);
		  else
		    rb_str_cat(str, "0", 1);
		}
		rb_str_cat(str, ",", 1);
	}
	
	return str;
}
/*

  After creating a semacode, it is possible to reuse the semacode object
  if you want to encode another URL. You should call the 'encode' method
  at any time to create a replacement semecode for the current object.
  
  It returns the semacode matrix as an array of arrays of boolean. The
  first element in the array is the top row, the last is the bottom row.
  
  Each row is also an array, containing boolean values. The length of each
  row is the same as the semacode width, and the number of rows is the same
  as the semacode height.

*/
static VALUE
semacode_encode(VALUE self, VALUE message)
{
  semacode_t *semacode;
  
  if (!rb_respond_to(message, rb_intern ("to_s")))
      rb_raise(rb_eRuntimeError, "target must respond to 'to_s'");
  
  Data_Get_Struct(self, semacode_t, semacode);
  
  /* free previous string if that exists */
  if(semacode->data != NULL) {
    free(semacode->data);
    semacode->data == NULL;
  }
  
  /* do a new encoding */
  DATA_PTR(self) = encode_string(semacode, StringValueLen(message), StringValuePtr(message));

  return semacode_grid(semacode);
}

/*
  This function gives the encoding organized by rows and columns.
  
  It returns the semacode matrix as an array of arrays of boolean. The
  first element in the array is the top row, the last is the bottom row.
  
  Each row is also an array, containing boolean values. The length of each
  row is the same as the semacode width, and the number of rows is the same
  as the semacode height.

*/
static VALUE
semacode_data(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  if(semacode->data == NULL)
    return Qnil;
  else
    return semacode_grid(semacode);
}

/*  
  This returns the encoding string used to create the semacode.
*/
static VALUE
semacode_encoded(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  return rb_str_new2(semacode->encoding);
}

/*  
  This returns the width of the semacode.
*/
static VALUE
semacode_width(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  return INT2FIX(semacode->width);
}

/*  
  This returns the height of the semacode.
*/
static VALUE
semacode_height(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  return INT2FIX(semacode->height);
}

/*  
  This returns the length of the semacode. It is
  the same as the product of the height and the width.
*/
static VALUE
semacode_length(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  return INT2FIX(semacode->height * semacode->width);
}

/*  
  This returns the length of the raw underlying encoding
  representing the data, before padding, error correction
  or any other operations on the raw encoding.
*/
static VALUE
semacode_raw_encoded_length(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  return INT2FIX(semacode->raw_encoded_length);
}

/*  
  This returns the maximum number of characters that can
  be stored in a symbol of the given width and height. You
  can use this to decide if it is worth packing in extra
  information while keeping the symbol size the same.
*/
static VALUE
semacode_symbol_size(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  return INT2FIX(semacode->symbol_capacity);
}

/*  
  This returns the number of bytes that are being devoted to
  error correction.
*/
static VALUE
semacode_ecc_bytes(VALUE self)
{
  semacode_t *semacode;
  Data_Get_Struct(self, semacode_t, semacode);

  return INT2FIX(semacode->ecc_bytes);
}

void 
Init_semacode_native()
{
  rb_mSemacode = rb_define_module ("DataMatrix");
  rb_cEncoder = rb_define_class_under(rb_mSemacode, "Encoder", rb_cObject);
  
  rb_define_alloc_func(rb_cEncoder, semacode_allocate);
  
  rb_define_method(rb_cEncoder, "initialize", semacode_init, 1);
  rb_define_method(rb_cEncoder, "encode", semacode_encode, 1);  
  rb_define_method(rb_cEncoder, "to_a", semacode_data, 0);
  rb_define_method(rb_cEncoder, "data", semacode_data, 0);  
  rb_define_method(rb_cEncoder, "encoding", semacode_encoded, 0);    
  rb_define_method(rb_cEncoder, "to_s", semacode_to_s, 0);
  rb_define_method(rb_cEncoder, "to_str", semacode_to_s, 0);  
  rb_define_method(rb_cEncoder, "width", semacode_width, 0);    
  rb_define_method(rb_cEncoder, "height", semacode_height, 0);
  rb_define_method(rb_cEncoder, "length", semacode_length, 0);
  rb_define_method(rb_cEncoder, "size", semacode_length, 0);  
  rb_define_method(rb_cEncoder, "raw_encoded_length", semacode_raw_encoded_length, 0);    
  rb_define_method(rb_cEncoder, "symbol_size", semacode_symbol_size, 0);    
  rb_define_method(rb_cEncoder, "ecc_bytes", semacode_ecc_bytes, 0);    
}
