
#ifndef PHP_CONFIGER_H
#define PHP_CONFIGER_H

extern zend_module_entry configer_module_entry;
#define phpext_configer_ptr &configer_module_entry

#define PHP_CONFIGER_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_CONFIGER_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_CONFIGER_API __attribute__ ((visibility("default")))
#else
#	define PHP_CONFIGER_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/

ZEND_BEGIN_MODULE_GLOBALS(configer)
	char *conffile_path;
	char *namespace_name;
ZEND_END_MODULE_GLOBALS(configer)

/* Always refer to the globals in your function as CONFIGER_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define CONFIGER_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(configer, v)

#if defined(ZTS) && defined(COMPILE_DL_CONFIGER)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_CONFIGER_H */

