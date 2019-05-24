#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
/* letest configer - configer.c
 * a configuration toml plugin
 * copyright 2019 dixyes (Yun Dou) MIT license
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "toml.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_configer.h"

#define CONFIGER_DEBUG 0
#if CONFIGER_DEBUG
#define CONFIGER_PRINTF printf
#define CONFIGER_PERROR perror
#else
#define CONFIGER_PRINTF(...)
#define CONFIGER_PERROR(...)
#endif

// fuck this fucking macro
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

PHP_CONFIGER_API ZEND_DECLARE_MODULE_GLOBALS(configer)

HashTable * configer_config = NULL;
char * strBuf = NULL;
toml_table_t * tomlRoot = NULL;

static int le_configer;

// ini entry for path.
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("configer.conffile_path", "./config.toml", PHP_INI_ALL, OnUpdateString, conffile_path, zend_configer_globals, configer_globals)
    STD_PHP_INI_ENTRY("configer.namespace", "configer", PHP_INI_ALL, OnUpdateString, namespace_name, zend_configer_globals, configer_globals)
PHP_INI_END()

// use this function to fetch config.
PHP_CONFIGER_API PHP_FUNCTION(configerGetConfig)
{
    //RETVAL_STR(zend_string_init("key", strlen("key"), 1));
    RETVAL_ARR(configer_config);
    Z_ADDREF_P(return_value);
    //ZVAL_DUP(&ret, return_value);
}

// set default conffile path
static void php_configer_init_globals(zend_configer_globals *that_globals)
{
    that_globals->conffile_path = "./config.toml";
    that_globals->namespace_name = "configer";
}

inline static void parseVal(const char * raw, HashTable * dest,const char * key){

    if(key){CONFIGER_PRINTF("%s=>",key);}

    int boolVal = 0;
    int64_t intVal = 0;
    double doubleVal = (double)-1L; // NaN
    struct toml_timestamp_t tsVal;

    zval zv;
    if(0 == toml_rtos(raw, &strBuf)){
        CONFIGER_PRINTF("String \"%s\"\n",strBuf);
        ZVAL_NEW_STR(&zv, zend_string_init(strBuf, strlen(strBuf), 1));
    }else{
        if(0 == toml_rtob(raw, &boolVal)){
            CONFIGER_PRINTF("Boolean %s\n", boolVal ? "True" : "False");
            ZVAL_BOOL(&zv, boolVal);
        }else if(0 == toml_rtoi(raw, &intVal)){
            CONFIGER_PRINTF("Integer %ld\n", intVal);
            ZVAL_LONG(&zv, intVal);
        }else if(0 == toml_rtod(raw, &doubleVal)){
            CONFIGER_PRINTF("Double %0.2f\n", doubleVal);
            ZVAL_DOUBLE(&zv, doubleVal);
        }else if(0 == toml_rtots(raw, &tsVal)){
            CONFIGER_PRINTF("Date: raw<%s>\n", raw);
            ZVAL_NEW_STR(&zv, zend_string_init(raw, strlen(raw), 1));
        }else{
            CONFIGER_PRINTF("<unknown: cant parse raw>\n");
            ZVAL_NULL(&zv);
        }
    }
    

    if(key){
        zend_hash_update(dest, zend_string_init(key, strlen(key), 1), &zv);
    }else{
        zend_hash_next_index_insert(dest, &zv);
    }
}

void toml2ht(toml_table_t * src, HashTable * dest);
void tomlArray2ht(toml_array_t * src, HashTable * dest){
    int j = 0;
    CONFIGER_PRINTF("Array(%d) {\n",toml_array_nelem(src));
    HashTable * newTable = NULL;
    zval zv ;
    switch (toml_array_kind(src)){
        case 't':
        {
            toml_table_t * t = NULL;
            while((t = toml_table_at(src, j++))){
                newTable = (HashTable *) malloc(sizeof(HashTable));
	            zend_hash_init(newTable, 0, NULL, ZVAL_PTR_DTOR, 1);
                //GC_ADDREF(newTable);
                CONFIGER_PRINTF("%d=>", j-1);
                toml2ht(t, newTable);
                ZVAL_ARR(&zv, newTable);
                zend_hash_next_index_insert(dest, &zv);
            }
            break;
        }
        case 'a':
        {
            toml_array_t * a = NULL;
            while((a = toml_array_at(src, j++))){
                newTable = (HashTable *) malloc(sizeof(HashTable));
	            zend_hash_init(newTable, 0, NULL, ZVAL_PTR_DTOR, 1);
                //GC_ADDREF(newTable);
                CONFIGER_PRINTF("%d=>", j-1);
                tomlArray2ht(a, newTable);
                ZVAL_ARR(&zv, newTable);
                zend_hash_next_index_insert(dest, &zv);
            }
            break;
        }
        case 'v':
        {
            const char * raw;
            while((raw = toml_raw_at(src, j++))){
                CONFIGER_PRINTF("%d=>", j-1);
                parseVal(raw, dest, NULL);
            }
            break;
        }
        default:
        CONFIGER_PRINTF("<fucked type array>\n");
    }
    CONFIGER_PRINTF("}\n");
}

void toml2ht(toml_table_t * src, HashTable * dest){

    HashTable * newTable=NULL;
    zval zv;

    const char * k;
    int i=0;
    CONFIGER_PRINTF("HashTable {\n");
    while ((k = toml_key_in(src ,i++))){
        // sub table
        toml_table_t * t = toml_table_in(src, k);
        if(NULL!=t){
            newTable = (HashTable *) malloc(sizeof(HashTable));
            zend_hash_init(newTable, 0, NULL, ZVAL_PTR_DTOR, 1);
            //GC_ADDREF(newTable);
            CONFIGER_PRINTF("%s=>", k);
            toml2ht(t, newTable);
            ZVAL_ARR(&zv, newTable);
            //zend_string * zs = zend_string_init(k, strlen(k), 1);
            //GC_ADDREF(zs);
            zend_hash_update(dest, zend_string_init(k, strlen(k), 1), &zv);
            continue;
        }
        // array
        toml_array_t * a = toml_array_in(src, k);
        if(NULL!=a){
            newTable = (HashTable *) malloc(sizeof(HashTable));
            zend_hash_init(newTable, 0, NULL, ZVAL_PTR_DTOR, 1);
            //GC_ADDREF(newTable);
            CONFIGER_PRINTF("%s=>", k);
            tomlArray2ht(a, newTable);
            ZVAL_ARR(&zv, newTable);
            //zend_string * zs = zend_string_init(k, strlen(k), 1);
            //GC_ADDREF(zs);
            zend_hash_update(dest, zend_string_init(k, strlen(k), 1), &zv);
            continue;
        }
        // value
        const char * raw = NULL;
        if(NULL == (raw = toml_raw_in(src, k))){
            CONFIGER_PRINTF("<unknown: no raw got>\n");
            continue;
        }
        parseVal(raw, dest, k);
        
    }
    CONFIGER_PRINTF("}\n");
}

PHP_CONFIGER_API PHP_MINIT_FUNCTION(configer)
{
#define tellwhat(reason) CONFIGER_PERROR(__FILE__ ":" LINE_STRING ":" reason)
#define then(condition,reason) if (!(condition)) { CONFIGER_PERROR(__FILE__ ":" LINE_STRING ":" reason); } else
	REGISTER_INI_ENTRIES();
    if (NULL==CONFIGER_G(namespace_name) || 1013<strlen(CONFIGER_G(namespace_name))){
        CONFIGER_PRINTF("namespace invalid\n");
        return SUCCESS;
    }
	configer_config=(HashTable *) malloc(sizeof(HashTable));
	zend_hash_init(configer_config, 0, NULL, ZVAL_PTR_DTOR, 1);
    //GC_ADDREF(configer_config);
	char * confPath = CONFIGER_G(conffile_path);
	struct stat confStat;
    int ret = stat(confPath, &confStat);if(0 != ret){ tellwhat("stat"); return SUCCESS; }
    int fd = open(confPath, O_RDONLY);if (-1 == fd){ tellwhat("open"); return SUCCESS; }
    strBuf = malloc(16384); // 16k static buffer TODO: realloc
    char * confBuf = emalloc(confStat.st_size+1);
    ssize_t red = read(fd, confBuf, confStat.st_size);then(confStat.st_size == red, "read"){
        confBuf[confStat.st_size] = 0;
        char * errBuf = emalloc(4096);
        tomlRoot = toml_parse(confBuf, errBuf, sizeof(errBuf));
        if(NULL==tomlRoot){
            CONFIGER_PRINTF("fucked parsing file %s due to:%s\n", confPath, errBuf);
        }else{
            // fake toml2ht behavier
            CONFIGER_PRINTF("wocao");
            zval zv;
            /*
            HashTable * hta=(HashTable *) malloc(sizeof(HashTable));
	        zend_hash_init(hta, 0, NULL, ZVAL_PTR_DTOR, 1);
            ZVAL_NEW_STR(&zv,zend_string_init("bbbb", strlen("bbbb"), 1));
            zend_hash_update(hta, zend_string_init("aaaa", strlen("aaaa"), 1), &zv);
            ZVAL_NEW_STR(&zv,zend_string_init("dddd", strlen("dddd"), 1));
            zend_hash_update(hta, zend_string_init("cccc", strlen("cccc"), 1), &zv);
            ZVAL_ARR(&zv, hta);
            zend_hash_update(configer_config, zend_string_init("ArrayA", strlen("ArrayA"), 1), &zv);

            HashTable * htb=(HashTable *) malloc(sizeof(HashTable));
            zend_hash_init(htb, 0, NULL, ZVAL_PTR_DTOR, 1);
            ZVAL_LONG(&zv, 1926);
            zend_hash_next_index_insert(htb, &zv);
            ZVAL_LONG(&zv, 8);
            zend_hash_next_index_insert(htb, &zv);
            ZVAL_LONG(&zv, 17);
            zend_hash_next_index_insert(htb, &zv);
            ZVAL_ARR(&zv, htb);
            zend_hash_update(configer_config, zend_string_init("ArrayB", strlen("ArrayB"), 1), &zv);
            */
            toml2ht(tomlRoot, configer_config);
            //toml_free(conf);
        }
        efree(errBuf);
    }
    efree(confBuf);
    free(strBuf);
    strBuf=NULL;

    // register getConfig here
    char fullFnName[1024];
    memset(fullFnName, 0, 1024);
    sprintf(fullFnName, "%s\\getConfig", CONFIGER_G(namespace_name));
    const zend_function_entry configer_functions[] = {
        ZEND_RAW_FENTRY(fullFnName, ZEND_FN(configerGetConfig), NULL, 0)
        PHP_FE_END
    };

    return zend_register_functions(NULL, configer_functions, NULL, MODULE_PERSISTENT);
#undef then
}

PHP_CONFIGER_API PHP_MSHUTDOWN_FUNCTION(configer)
{
	UNREGISTER_INI_ENTRIES();
	//zend_hash_destroy(configer_config);
	//FREE_HASHTABLE(configer_config);
    //toml_free(tomlRoot);
	return SUCCESS;
}

PHP_MINFO_FUNCTION(configer)
{
	// seems we needn't this
	//php_info_priint_table_start();
	//php_info_print_table_header(2, "configer", "enabled");
	//php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

/*
const zend_function_entry configer_functions[] = {
	PHP_FE_END
};
*/

PHP_CONFIGER_API zend_module_entry configer_module_entry = {
	STANDARD_MODULE_HEADER,
	"configer",
	NULL,//configer_functions,
	PHP_MINIT(configer),
	PHP_MSHUTDOWN(configer),
	NULL,
	NULL,
	PHP_MINFO(configer),
	PHP_CONFIGER_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CONFIGER
#ifdef ZTS
PHP_CONFIGER_API ZEND_TSRMLS_CACHE_DEFINE()
#endif
PHP_CONFIGER_API ZEND_GET_MODULE(configer)
#endif
