%module icsneojava
%include <windows.i>
%include <typemaps.i>
%include <stdint.i>
%include <carrays.i>
%include <various.i>
%include <java.i>
%include <arrays_java.i>

#define DLLExport

%typemap(jni) uint8_t const *data "jbyteArray"
%typemap(jtype) uint8_t const *data "byte[]"
%typemap(jstype) uint8_t const *data "byte[]"
%typemap(javain) uint8_t const *data "$javainput"

%typemap(javaout) uint8_t const *data {
    return $jnicall;
}

%typemap(out) uint8_t const *data %{
    $result = (*jenv)->NewByteArray(jenv, (int) arg1->length);
    (*jenv)->SetByteArrayRegion(jenv, $result, 0, (int) arg1->length, $1);
%}

%typemap(in) uint8_t const *data %{
    bool isCopy;
    arg2 = (*jenv)->GetByteArrayElements(jenv, jarg2, &isCopy);
%}

%typemap(jni) char *str "jobject"
%typemap(jtype) char *str "StringBuffer"
%typemap(jstype) char *str "StringBuffer"

/* How to convert Java(JNI) type to requested C type */
%typemap(in) char *str {

  $1 = NULL;
  if($input != NULL) {
    /* Get the String from the StringBuffer */
    jmethodID setLengthID;
    jclass strClass = (*jenv)->GetObjectClass(jenv, $input);
    jmethodID toStringID = (*jenv)->GetMethodID(jenv, strClass, "toString", "()Ljava/lang/String;");
    jstring js = (jstring) (*jenv)->CallObjectMethod(jenv, $input, toStringID);

    /* Convert the String to a C string */
    const char *pCharStr = (*jenv)->GetStringUTFChars(jenv, js, 0);

    /* Take a copy of the C string as the typemap is for a non const C string */
    jmethodID capacityID = (*jenv)->GetMethodID(jenv, strClass, "capacity", "()I");
    jint capacity = (*jenv)->CallIntMethod(jenv, $input, capacityID);
    $1 = (char *) malloc(capacity+1);
    strcpy($1, pCharStr);

    /* Release the UTF string we obtained with GetStringUTFChars */
    (*jenv)->ReleaseStringUTFChars(jenv,  js, pCharStr);

    /* Zero the original StringBuffer, so we can replace it with the result */
    setLengthID = (*jenv)->GetMethodID(jenv, strClass, "setLength", "(I)V");
    (*jenv)->CallVoidMethod(jenv, $input, setLengthID, (jint) 0);
  }
}

/* How to convert the C type to the Java(JNI) type */
%typemap(argout) char *str {

  if($1 != NULL) {
    /* Append the result to the empty StringBuffer */
    jstring newString = (*jenv)->NewStringUTF(jenv, $1);
    jclass strClass = (*jenv)->GetObjectClass(jenv, $input);
    jmethodID appendStringID = (*jenv)->GetMethodID(jenv, strClass, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;");
    (*jenv)->CallObjectMethod(jenv, $input, appendStringID, newString);

    /* Clean up the string object, no longer needed */
    free($1);
    $1 = NULL;
  }
}

/* Prevent the default freearg typemap from being used */
%typemap(freearg) char *str ""

/* Convert the jstype to jtype typemap type */
%typemap(javain) char *str "$javainput"

%{
#include "icsneo/icsneoc.h"
%}

%apply int *INOUT {size_t *};

%ignore icsneo_addMessageCallback;
%ignore icsneo_removeMessageCallback;
%ignore icsneo_addEventCallback;
%ignore icsneo_removeEventCallback;

%include "icsneo/icsneoc.h"
%include "icsneo/device/neodevice.h"
%include "icsneo/communication/message/neomessage.h"
%include "icsneo/device/devicetype.h"
%include "icsneo/api/version.h"
%include "icsneo/api/event.h"
%include "icsneo/communication/network.h"

%inline %{
static neomessage_can_t* neomessage_can_t_cast(neomessage_t* msg) {
	return (neomessage_can_t*) msg;
}

static neomessage_eth_t* neomessage_eth_t_cast(neomessage_t* msg) {
	return (neomessage_eth_t*) msg;
}

static neomessage_t* from_can_neomessage_t_cast(neomessage_can_t* msg) {
	return (neomessage_t*) msg;
}

static neomessage_t* from_eth_neomessage_t_cast(neomessage_eth_t* msg) {
	return (neomessage_t*) msg;
}
%}

%array_functions(neodevice_t, neodevice_t_array);
%array_functions(neoevent_t, neoevent_t_array);
%array_functions(neomessage_t, neomessage_t_array);