%module icsneocsharp
%include <windows.i>
%include <typemaps.i>
%include <stdint.i>
%include <carrays.i>
%include <arrays_csharp.i>

#define DLLExport

%typemap(ctype) uint8_t const *data "unsigned char *" 
%typemap(imtype, out="System.IntPtr") uint8_t const *data "byte[]"
%typemap(cstype) uint8_t const *data "byte[]"

%typemap(in) uint8_t const *data %{
	$1 = $input;
%}

%typemap(csvarin) uint8_t const *data %{
	set {
		icsneocsharpPINVOKE.$csclazznamedata_set(swigCPtr, value);
	}
%}

%typemap(csvarout, excode=SWIGEXCODE2) uint8_t const *data %{
	get {
		byte[] ret = new byte[this.length]; 
        System.IntPtr data = $imcall; 
        System.Runtime.InteropServices.Marshal.Copy(data, ret, 0, (int)this.length)$excode;
		return ret; 
	}
%}

%typemap(ctype) char *str "char *"
%typemap(imtype) char *str "System.Text.StringBuilder"
%typemap(cstype) char *str "System.Text.StringBuilder" 

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