// Public Domain. See "unlicense" statement at the end of this file.

// ABOUT
//
// dr_mtl is a library for loading materials for use in 2D or 3D graphics. But it's a bit different to
// what you may expect. It does not reprsent materials as a static data, but rather as a series of
// instructions that interface with a set of inputs. Indeed, this library is more like a compiler - you
// specify an input file (such as a Wavefront MTL file), compile it into an intermediate bytecode
// represenation, and then run it through a code generator to generate shader code such as GLSL, HLSL,
// Spir-V, etc.
//
//
//
// USAGE
//
// This is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_MTL_IMPLEMENTATION
//   #include "dr_mtl.h"
//
// You can then #include dr_mtl.h in other parts of the program as you would with any other header file.
//
//
//
// OPTIONS
//
// #define DRMTL_NO_MTL_COMPILER
//   Disables the Wavefront MTL compiler.
//
// #define DRMTL_NO_GLSL_CODEGEN
//   Disables the GLSL code generator.
//
//
//
// TODO
// - Add more documentation.
// - Add a demo to demonstrate the awesomeness of this library.
// - Add trigonometric instructions.

#ifndef dr_mtl_h
#define dr_mtl_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4201)   // Non-standard extension used: nameless struct/union.
#endif


#define DRMTL_MAGIC_NUMBER        0x81DF7405
#define DRMTL_CURRENT_VERSION     1

#define DRMTL_INPUT_DESC_CONSTI   ((unsigned char)-2)
#define DRMTL_INPUT_DESC_CONSTF   ((unsigned char)-1)
#define DRMTL_INPUT_DESC_VARX     ((unsigned char)0)
#define DRMTL_INPUT_DESC_VARY     ((unsigned char)1)
#define DRMTL_INPUT_DESC_VARZ     ((unsigned char)2)
#define DRMTL_INPUT_DESC_VARW     ((unsigned char)3)

#define DRMTL_MAX_IDENTIFIER_NAME 28
#define DRMTL_MAX_CHANNEL_NAME    28
#define DRMTL_MAX_PROPERTY_NAME   28

#define DRMTL_MAX_INPUT_PATH      252
#define DRMTL_MAX_PROPERTY_PATH   224


typedef unsigned char drmtl_uint8;
typedef unsigned int  drmtl_uint32;

typedef struct drmtl_material drmtl_material;


/// The various data type available to the material.
typedef enum
{
    drmtl_type_float   = 1,
    drmtl_type_float2  = 2,
    drmtl_type_float3  = 3,
    drmtl_type_float4  = 4,
    drmtl_type_int     = 5,
    drmtl_type_int2    = 6,
    drmtl_type_int3    = 7,
    drmtl_type_int4    = 8,
    drmtl_type_tex1d   = 9,
    drmtl_type_tex2d   = 10,
    drmtl_type_tex3d   = 11,
    drmtl_type_texcube = 12,
    drmtl_type_bool    = 13

} drmtl_type;

/// The various run-time opcodes.
typedef enum
{
    //////////////////////////
    // Assignment

    // mov
    drmtl_opcode_movf1 = 0x00000001,
    drmtl_opcode_movf2 = 0x00000002,
    drmtl_opcode_movf3 = 0x00000003,
    drmtl_opcode_movf4 = 0x00000004,
    drmtl_opcode_movi1 = 0x00000005,
    drmtl_opcode_movi2 = 0x00000006,
    drmtl_opcode_movi3 = 0x00000007,
    drmtl_opcode_movi4 = 0x00000008,


    //////////////////////////
    // Arithmetic

    // add
    drmtl_opcode_addf1 = 0x00001001,
    drmtl_opcode_addf2 = 0x00001002,
    drmtl_opcode_addf3 = 0x00001003,
    drmtl_opcode_addf4 = 0x00001004,
    drmtl_opcode_addi1 = 0x00001005,
    drmtl_opcode_addi2 = 0x00001006,
    drmtl_opcode_addi3 = 0x00001007,
    drmtl_opcode_addi4 = 0x00001008,

    // sub
    drmtl_opcode_subf1 = 0x00001101,
    drmtl_opcode_subf2 = 0x00001102,
    drmtl_opcode_subf3 = 0x00001103,
    drmtl_opcode_subf4 = 0x00001104,
    drmtl_opcode_subi1 = 0x00001105,
    drmtl_opcode_subi2 = 0x00001106,
    drmtl_opcode_subi3 = 0x00001107,
    drmtl_opcode_subi4 = 0x00001108,

    // mul
    drmtl_opcode_mulf1 = 0x00001201,
    drmtl_opcode_mulf2 = 0x00001202,
    drmtl_opcode_mulf3 = 0x00001203,
    drmtl_opcode_mulf4 = 0x00001204,
    drmtl_opcode_muli1 = 0x00001205,
    drmtl_opcode_muli2 = 0x00001206,
    drmtl_opcode_muli3 = 0x00001207,
    drmtl_opcode_muli4 = 0x00001208,

    // div
    drmtl_opcode_divf1 = 0x00001301,
    drmtl_opcode_divf2 = 0x00001302,
    drmtl_opcode_divf3 = 0x00001303,
    drmtl_opcode_divf4 = 0x00001304,
    drmtl_opcode_divi1 = 0x00001305,
    drmtl_opcode_divi2 = 0x00001306,
    drmtl_opcode_divi3 = 0x00001307,
    drmtl_opcode_divi4 = 0x00001308,

    // pow
    drmtl_opcode_powf1 = 0x00001401,
    drmtl_opcode_powf2 = 0x00001402,
    drmtl_opcode_powf3 = 0x00001403,
    drmtl_opcode_powf4 = 0x00001404,
    drmtl_opcode_powi1 = 0x00001405,
    drmtl_opcode_powi2 = 0x00001406,
    drmtl_opcode_powi3 = 0x00001407,
    drmtl_opcode_powi4 = 0x00001408,


    //////////////////////////
    // Textures

    // tex
    drmtl_opcode_tex1    = 0x00002001,
    drmtl_opcode_tex2    = 0x00002002,
    drmtl_opcode_tex3    = 0x00002003,
    drmtl_opcode_texcube = 0x00002004,


    //////////////////////////
    // Miscellaneous

    // var
    drmtl_opcode_var   = 0x00003000,

    // ret
    drmtl_opcode_retf1 = 0x00003001,
    drmtl_opcode_retf2 = 0x00003002,
    drmtl_opcode_retf3 = 0x00003003,
    drmtl_opcode_retf4 = 0x00003004,
    drmtl_opcode_reti1 = 0x00003005,
    drmtl_opcode_reti2 = 0x00003006,
    drmtl_opcode_reti3 = 0x00003007,
    drmtl_opcode_reti4 = 0x00003008

} drmtl_opcode;


/// Structure containing information about an identifier. An identifier contains a type (float, float2, etc.) and a name. The
/// total size of this structure is 32 bytes. The name is null terminated.
typedef struct
{
    /// The type of the identifier.
    drmtl_type type;

    /// The name of the identifier.
    char name[DRMTL_MAX_IDENTIFIER_NAME];

} drmtl_identifier;


/// Structure containing information about an input variable.
typedef struct
{
    /// The index into the identifier table that this input variable is identified by.
    unsigned int identifierIndex;

    /// The default value of the input variable.
    union
    {
		struct
        {
            float x;
        } f1;
        struct
        {
            float x;
            float y;
        } f2;
        struct
        {
            float x;
            float y;
            float z;
        } f3;
        struct
        {
            float x;
            float y;
            float z;
            float w;
        } f4;

        struct
        {
            int x;
        } i1;
        struct
        {
            int x;
            int y;
        } i2;
        struct
        {
            int x;
            int y;
            int z;
        } i3;
        struct
        {
            int x;
            int y;
            int z;
            int w;
        } i4;

        struct
        {
            char value[DRMTL_MAX_INPUT_PATH];	// Enough room for a path, but less to keep the total size of the structure at 256 bytes. Null terminated.
        } path;


        struct
        {
            drmtl_uint32 x;
            drmtl_uint32 y;
            drmtl_uint32 z;
            drmtl_uint32 w;
        } raw4;
    };

} drmtl_input;


typedef struct
{
    /// The return type of the channel.
    drmtl_type type;

    /// The name of the channel. Null terminated.
    char name[DRMTL_MAX_CHANNEL_NAME];

} drmtl_channel;


/// An instruction input value. An input value to an instruction can usually be a constant or the identifier index of the
/// applicable variable.
typedef union
{
    /// The identifier index of the applicable variable.
    unsigned int id;

    /// The constant value, as a float.
    float valuef;

    /// The constant value, as an integer.
    int valuei;

}drmtl_instruction_input;


/// Structure used for describing an instructions input data.
typedef struct
{
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;

} drmtl_instruction_input_descriptor;

/// Structure containing information about an instruction.
typedef struct
{
    /// The instruction's opcode.
    drmtl_opcode opcode;

    /// The instruction's data.
	union
	{
        // mov data.
		struct
		{
            drmtl_instruction_input_descriptor inputDesc;
			drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
            unsigned int output;
		} mov;


        // add data.
        struct
        {
            drmtl_instruction_input_descriptor inputDesc;
            drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
            unsigned int output;
        } add;

        // sub data.
        struct
        {
            drmtl_instruction_input_descriptor inputDesc;
            drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
            unsigned int output;
        } sub;

        // mul data.
        struct
        {
            drmtl_instruction_input_descriptor inputDesc;
            drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
            unsigned int output;
        } mul;

        // div data.
        struct
        {
            drmtl_instruction_input_descriptor inputDesc;
            drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
            unsigned int output;
        } div;

        // pow data.
        struct
        {
            drmtl_instruction_input_descriptor inputDesc;
            drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
            unsigned int output;
        } pow;


        // tex data.
        struct
        {
            drmtl_instruction_input_descriptor inputDesc;
            drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
            unsigned int texture;
            unsigned int output;
        } tex;


        // ret data.
        struct
        {
            drmtl_instruction_input_descriptor inputDesc;
            drmtl_instruction_input inputX;
            drmtl_instruction_input inputY;
            drmtl_instruction_input inputZ;
            drmtl_instruction_input inputW;
        } ret;


        // var data
        struct
        {
            //drmtl_type type;
            unsigned int identifierIndex;
        } var;


        // Ensures the size of the instruction is always 32 bytes.
        struct
        {
            drmtl_uint8 _unused[32];
        } unused;
	};

} drmtl_instruction;


typedef struct
{
    /// The type of the property.
    drmtl_type type;

    /// The name of the property.
    char name[DRMTL_MAX_PROPERTY_NAME];

    /// The default value of the input variable.
    union
    {
        struct
        {
            float x;
        } f1;
        struct
        {
            float x;
            float y;
        } f2;
        struct
        {
            float x;
            float y;
            float z;
        } f3;
        struct
        {
            float x;
            float y;
            float z;
            float w;
        } f4;

        struct
        {
            int x;
        } i1;
        struct
        {
            int x;
            int y;
        } i2;
        struct
        {
            int x;
            int y;
            int z;
        } i3;
        struct
        {
            int x;
            int y;
            int z;
            int w;
        } i4;

        struct
        {
            char value[DRMTL_MAX_PROPERTY_PATH];	// Enough room for a path, but less to keep the total size of the structure at 256 bytes. Null terminated.
        } path;

        struct
        {
            int x;
        } b1;
    };

} drmtl_property;


/// Structure containing the header information of the material.
typedef struct
{
    /// The magic number: 0x81DF7405
    unsigned int magic;

    /// The version number. There is currently only a single version - version 1. In the future there may be other versions
    /// which will affect how the file is formatted.
    unsigned int version;


    /// The size in bytes of an identifier.
    unsigned int identifierSizeInBytes;

    /// The size in bytes of an input variable.
    unsigned int inputSizeInBytes;

    /// The size of a channel header, in bytes.
    unsigned int channelHeaderSizeInBytes;

    /// The size in bytes of an instruction.
    unsigned int instructionSizeInBytes;

    /// The size in bytes of a property.
    unsigned int propertySizeInBytes;


    /// The total number of identifiers.
    unsigned int identifierCount;

    /// The total number of private input variables.
    unsigned int privateInputCount;

    /// The total number of public input variables.
    unsigned int publicInputCount;

    /// The total number of channels.
    unsigned int channelCount;

    /// The total number of properties.
    unsigned int propertyCount;


    /// The offset of the identifiers.
    unsigned int identifiersOffset;

    /// The offset of the input variables.
    unsigned int inputsOffset;

    /// The offset of the channels.
    unsigned int channelsOffset;

    /// The offset of the properties.
    unsigned int propertiesOffset;


} drmtl_header;

typedef struct
{
    /// The channel information.
    drmtl_channel channel;

    /// The instruction count of the channel.
    unsigned int instructionCount;

} drmtl_channel_header;


/// Structure containing the definition of the material.
struct drmtl_material
{
    /// A pointer to the raw data. This will at least be the size of drmtl_header (128 bytes, as of version 1).
    drmtl_uint8* pRawData;

    /// The size of the data, in bytes.
    unsigned int sizeInBytes;

    /// The size of the buffer, in bytes. This is used to determine when the buffer needs to be inflated.
    unsigned int bufferSizeInBytes;


    /// The current stage of the construction process of the material. A material must be constructed in order, so this
    /// keeps track of the current stage to ensure the proper errors are returned.
    unsigned int currentStage;

    /// The offset of the current channel. This is needed to we can determine which bytes need to be updated as
    /// instructions are added to the channel.
    unsigned int currentChannelOffset;


    /// Whether or not the material data is owned by this library. When this is set to false, the library will
    /// never modify the original pointer.
    bool ownsRawData;
};



////////////////////////////////////////////////////////
// Low-Level APIs
//
// Use these APIs to work on materials directly. Note that these are intentionally restrictive in that things can
// be added to the material, however they can never be removed. In addition, everything must be added in order which
// means identifiers need to be added first, followed by private inputs, followed by public inputs, followed by
// channels, followed by properties.
//
// Use these to construct a material after everything has been processed at a higher level.

/// Initializes the given material.
///
/// @param pMaterial [in] A pointer to the material to initialize.
///
/// @return True if the material is initialized successfully; false otherwise.
bool drmtl_init(drmtl_material* pMaterial);
bool drmtl_initfromexisting(drmtl_material* pMaterial, const void* pRawData, unsigned int dataSizeInBytes);
bool drmtl_initfromexisting_nocopy(drmtl_material* pMaterial, const void* pRawData, unsigned int dataSizeInBytes);

/// Uninitializes the given material.
///
/// @param pMaterial [in] A pointer to the material to uninitialize.
void drmtl_uninit(drmtl_material* pMaterial);


/// Retrieve a pointer to the header information.
drmtl_header* drmtl_getheader(drmtl_material* pMaterial);


/// Appends an identifier to the end of the identifier list. Use drmtl_getidentifiercount() to determine it's index.
///
/// @param pMaterial [in] A pointer to the material to append the identifier to.
bool drmtl_appendidentifier(drmtl_material* pMaterial, drmtl_identifier identifier, unsigned int* indexOut);

/// Appends a private input variable.
bool drmtl_appendprivateinput(drmtl_material* pMaterial, drmtl_input input);

/// Appends a public input variable.
bool drmtl_appendpublicinput(drmtl_material* pMaterial, drmtl_input input);

/// Begins a new channel.
///
/// @remarks
///     Any instructions that are appended from now on will be part of this channel until another channel is begun.
///     @par
///     The end of the channel is marked when a new channel is appended or a property begins.
bool drmtl_appendchannel(drmtl_material* pMaterial, drmtl_channel channelHeader);

/// Appends an instruction to the most recently appended channel.
bool drmtl_appendinstruction(drmtl_material* pMaterial, drmtl_instruction instruction);

/// Append a property.
bool drmtl_appendproperty(drmtl_material* pMaterial, drmtl_property prop);


/// Retrieves a pointer to the channel header by it's index.
///
/// @remarks
///     This runs in linear time.
drmtl_channel_header* drmtl_getchannelheaderbyindex(drmtl_material* pMaterial, unsigned int channelIndex);

/// Retrieves a pointer to the channel header by it's name.
///
/// @remarks
///     This runs in linear time.
drmtl_channel_header* drmtl_getchannelheaderbyname(drmtl_material* pMaterial, const char* channelName);


/// Retrieves a pointer to the buffer containing the list of identifiers.
drmtl_identifier* drmtl_getidentifiers(drmtl_material* pMaterial);
drmtl_identifier* drmtl_getidentifier(drmtl_material* pMaterial, unsigned int index);

/// Retrieves the number of identifiers defined by the given material.
unsigned int drmtl_getidentifiercount(drmtl_material* pMaterial);


/// Retrieves the number of private + public input variables.
unsigned int drmtl_getinputcount(drmtl_material* pMaterial);

/// Retrieves the input variable by it's index.
drmtl_input* drmtl_getinputbyindex(drmtl_material* pMaterial, unsigned int index);

/// Retrieves the number of private input variables.
unsigned int drmtl_getprivateinputcount(drmtl_material* pMaterial);

/// Retrieves the private input variable by it's index.
drmtl_input* drmtl_getprivateinputbyindex(drmtl_material* pMaterial, unsigned int index);

/// Retrieves the number of public input variables.
unsigned int drmtl_getpublicinputcount(drmtl_material* pMaterial);

/// Retrieves the public input variable by it's index.
drmtl_input* drmtl_getpublicinputbyindex(drmtl_material* pMaterial, unsigned int index);


/// Retrieves the number of properties.
unsigned int drmtl_getpropertycount(drmtl_material* pMaterial);

/// Retrieves a property by it's index.
drmtl_property* drmtl_getpropertybyindex(drmtl_material* pMaterial, unsigned int index);

/// Retrieves a properties by it's name.
///
/// @remarks
///     This is case-sensitive.
drmtl_property* drmtl_getpropertybyname(drmtl_material* pMaterial, const char* name);



////////////////////////////////////////////////////////
// Mid-Level APIs

/// Helper for creating an identifier.
drmtl_identifier drmtl_identifier_float(const char* name);
drmtl_identifier drmtl_identifier_float2(const char* name);
drmtl_identifier drmtl_identifier_float3(const char* name);
drmtl_identifier drmtl_identifier_float4(const char* name);
drmtl_identifier drmtl_identifier_int(const char* name);
drmtl_identifier drmtl_identifier_int2(const char* name);
drmtl_identifier drmtl_identifier_int3(const char* name);
drmtl_identifier drmtl_identifier_int4(const char* name);
drmtl_identifier drmtl_identifier_tex2d(const char* name);

/// Helper for creating an input variable.
drmtl_input drmtl_input_float(unsigned int identifierIndex, float x);
drmtl_input drmtl_input_float2(unsigned int identifierIndex, float x, float y);
drmtl_input drmtl_input_float3(unsigned int identifierIndex, float x, float y, float z);
drmtl_input drmtl_input_float4(unsigned int identifierIndex, float x, float y, float z, float w);
drmtl_input drmtl_input_int(unsigned int identifierIndex, int x);
drmtl_input drmtl_input_int2(unsigned int identifierIndex, int x, int y);
drmtl_input drmtl_input_int3(unsigned int identifierIndex, int x, int y, int z);
drmtl_input drmtl_input_int4(unsigned int identifierIndex, int x, int y, int z, int w);
drmtl_input drmtl_input_tex(unsigned int identifierIndex, const char* path);

/// Helper for creating a channel.
drmtl_channel drmtl_channel_float(const char* name);
drmtl_channel drmtl_channel_float2(const char* name);
drmtl_channel drmtl_channel_float3(const char* name);
drmtl_channel drmtl_channel_float4(const char* name);
drmtl_channel drmtl_channel_int(const char* name);
drmtl_channel drmtl_channel_int2(const char* name);
drmtl_channel drmtl_channel_int3(const char* name);
drmtl_channel drmtl_channel_int4(const char* name);

/// Helper for creating an instruction. These are heavily simplified and more complex setups are possible using lower level APIs.
drmtl_instruction drmtl_movf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_movf1_c1(unsigned int outputIdentifierIndex, float x);
drmtl_instruction drmtl_movf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_movf2_c2(unsigned int outputIdentifierIndex, float x, float y);
drmtl_instruction drmtl_movf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_movf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z);
drmtl_instruction drmtl_movf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_movf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w);

drmtl_instruction drmtl_addf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_addf1_c1(unsigned int outputIdentifierIndex, float x);
drmtl_instruction drmtl_addf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_addf2_c2(unsigned int outputIdentifierIndex, float x, float y);
drmtl_instruction drmtl_addf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_addf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z);
drmtl_instruction drmtl_addf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_addf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w);

drmtl_instruction drmtl_subf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_subf1_c1(unsigned int outputIdentifierIndex, float x);
drmtl_instruction drmtl_subf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_subf2_c2(unsigned int outputIdentifierIndex, float x, float y);
drmtl_instruction drmtl_subf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_subf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z);
drmtl_instruction drmtl_subf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_subf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w);

drmtl_instruction drmtl_mulf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_mulf1_c1(unsigned int outputIdentifierIndex, float x);
drmtl_instruction drmtl_mulf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_mulf2_c2(unsigned int outputIdentifierIndex, float x, float y);
drmtl_instruction drmtl_mulf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_mulf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z);
drmtl_instruction drmtl_mulf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_mulf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w);
drmtl_instruction drmtl_mulf4_v3v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndexXYZ, unsigned int inputIdentifierIndexW);
drmtl_instruction drmtl_mulf4_v3c1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex, float w);
drmtl_instruction drmtl_mulf4_v2c2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex, float z, float w);
drmtl_instruction drmtl_mulf4_v1c3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex, float y, float z, float w);
drmtl_instruction drmtl_mulf4_v1v1v1v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndexX, unsigned int inputIdentifierIndexY, unsigned int inputIdentifierIndexZ, unsigned int inputIdentifierIndexW);

drmtl_instruction drmtl_divf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_divf1_c1(unsigned int outputIdentifierIndex, float x);
drmtl_instruction drmtl_divf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_divf2_c2(unsigned int outputIdentifierIndex, float x, float y);
drmtl_instruction drmtl_divf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_divf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z);
drmtl_instruction drmtl_divf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex);
drmtl_instruction drmtl_divf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w);

drmtl_instruction drmtl_tex2(unsigned int outputIdentifierIndex, unsigned int textureIdentifierIndex, unsigned int texcoordIdentifierIndex);
drmtl_instruction drmtl_var(unsigned int identifierIndex);
drmtl_instruction drmtl_retf1(unsigned int identifierIndex);
drmtl_instruction drmtl_retf2(unsigned int identifierIndex);
drmtl_instruction drmtl_retf3(unsigned int identifierIndex);
drmtl_instruction drmtl_retf4(unsigned int identifierIndex);
drmtl_instruction drmtl_retf1_c1(float x);
drmtl_instruction drmtl_retf2_c2(float x, float y);
drmtl_instruction drmtl_retf3_c3(float x, float y, float z);
drmtl_instruction drmtl_retf4_c4(float x, float y, float z, float w);
drmtl_instruction drmtl_reti1(unsigned int identifierIndex);
drmtl_instruction drmtl_reti2(unsigned int identifierIndex);
drmtl_instruction drmtl_reti3(unsigned int identifierIndex);
drmtl_instruction drmtl_reti4(unsigned int identifierIndex);
drmtl_instruction drmtl_reti1_c1(int x);
drmtl_instruction drmtl_reti2_c2(int x, int y);
drmtl_instruction drmtl_reti3_c3(int x, int y, int z);
drmtl_instruction drmtl_reti4_c4(int x, int y, int z, int w);

/// Helper for creating a property.
drmtl_property drmtl_property_float(const char* name, float x);
drmtl_property drmtl_property_float2(const char* name, float x, float y);
drmtl_property drmtl_property_float3(const char* name, float x, float y, float z);
drmtl_property drmtl_property_float4(const char* name, float x, float y, float z, float w);
drmtl_property drmtl_property_int(const char* name, int x);
drmtl_property drmtl_property_int2(const char* name, int x, int y);
drmtl_property drmtl_property_int3(const char* name, int x, int y, int z);
drmtl_property drmtl_property_int4(const char* name, int x, int y, int z, int w);
drmtl_property drmtl_property_bool(const char* name, bool value);




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Compilers
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef DRMTL_NO_MTL_COMPILER
/// Compiles a Wavefront MTL file.
///
/// @param pMaterial [in] A pointer to the destination material.
///
/// @remarks
///     This will compile the material at the first occurance of the "newmtl" statement, and will end at either the next
///     occurance of "newmtl" of when the input buffer has been exhausted.
///     @par
///     This will initialize the material, so ensure that you have not already initialized it before calling this. If this
///     returns successfully, call drmtl_uninit() to uninitialize the material.
///     @par
///     MTL files require texture coordinates in order to know how to select the appropriate sample from textures. The
///     of the variable to use is specified in "texcoordInputName", and assumed to have at least 2 components (x and y).
bool drmtl_compile_wavefront_mtl(drmtl_material* pMaterial, const char* mtlData, size_t mtlDataSizeInBytes, const char* texcoordInputName);
#endif




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Code Generators
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef DRMTL_NO_GLSL_CODEGEN
/// Generates GLSL code for the channel with the given name.
bool drmtl_codegen_glsl_channel(drmtl_material* pMaterial, const char* channelName, char* codeOut, size_t codeOutSizeInBytes, size_t* pBytesWrittenOut);

/// Generates GLSL code for the uniform variables as defined by the material's public input variables.
bool drmtl_codegen_glsl_uniforms(drmtl_material* pMaterial, char* codeOut, size_t codeOutSizeInBytes, size_t* pBytesWritteOut);
#endif





#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif  //dr_mtl_h


///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_MTL_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#if defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-align"
#endif


// When constructing the material's raw data, memory is allocated in blocks of this amount. This must be at least 256.
#define DRMTL_CHUNK_SIZE              4096

#define DRMTL_STAGE_IDS               0
#define DRMTL_STAGE_PRIVATE_INPUTS    1
#define DRMTL_STAGE_PUBLIC_INPUTS     2
#define DRMTL_STAGE_CHANNELS          3
#define DRMTL_STAGE_PROPERTIES        4
#define DRMTL_STAGE_COMPLETE          UINT_MAX


////////////////////////////////////////////////////////
// Utilities

// strcpy()
static int drmtl_strcpy(char* dst, size_t dstSizeInBytes, const char* src)
{
#if defined(_MSC_VER)
    return strcpy_s(dst, dstSizeInBytes, src);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    char* iDst = dst;
    const char* iSrc = src;
    size_t remainingSizeInBytes = dstSizeInBytes;
    while (remainingSizeInBytes > 0 && iSrc[0] != '\0')
    {
        iDst[0] = iSrc[0];

        iDst += 1;
        iSrc += 1;
        remainingSizeInBytes -= 1;
    }

    if (remainingSizeInBytes > 0) {
        iDst[0] = '\0';
    } else {
        dst[0] = '\0';
        return ERANGE;
    }

    return 0;
#endif
}



/// Inflates the materials data buffer by DRMTL_CHUNK_SIZE.
bool _drmtl_inflate(drmtl_material* pMaterial);


bool drmtl_init(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        assert(DRMTL_CHUNK_SIZE >= 256);

        pMaterial->pRawData = (drmtl_uint8*)malloc(DRMTL_CHUNK_SIZE);
        if (pMaterial->pRawData != NULL)
        {
            pMaterial->sizeInBytes          = sizeof(drmtl_header);
            pMaterial->bufferSizeInBytes    = DRMTL_CHUNK_SIZE;
            pMaterial->currentStage         = DRMTL_STAGE_IDS;
            pMaterial->currentChannelOffset = 0;
            pMaterial->ownsRawData          = 1;

            drmtl_header* pHeader = drmtl_getheader(pMaterial);
            assert(pHeader != NULL);

            pHeader->magic                    = DRMTL_MAGIC_NUMBER;
            pHeader->version                  = DRMTL_CURRENT_VERSION;
            pHeader->identifierSizeInBytes    = sizeof(drmtl_identifier);
            pHeader->inputSizeInBytes         = sizeof(drmtl_input);
            pHeader->channelHeaderSizeInBytes = sizeof(drmtl_channel_header);
            pHeader->instructionSizeInBytes   = sizeof(drmtl_instruction);
            pHeader->propertySizeInBytes      = sizeof(drmtl_property);
            pHeader->identifierCount          = 0;
            pHeader->privateInputCount        = 0;
            pHeader->publicInputCount         = 0;
            pHeader->channelCount             = 0;
            pHeader->propertyCount            = 0;
            pHeader->identifiersOffset        = pMaterial->sizeInBytes;
            pHeader->inputsOffset             = pMaterial->sizeInBytes;
            pHeader->channelsOffset           = pMaterial->sizeInBytes;
            pHeader->propertiesOffset         = pMaterial->sizeInBytes;

            return 1;
        }
    }

    return 0;
}

bool drmtl_initfromexisting(drmtl_material* pMaterial, const void* pRawData, unsigned int dataSizeInBytes)
{
    if (pMaterial != NULL)
    {
        if (pRawData != NULL && dataSizeInBytes >= sizeof(drmtl_header))
        {
            if (((drmtl_header*)pMaterial->pRawData)->magic == DRMTL_MAGIC_NUMBER)
            {
                pMaterial->pRawData = (drmtl_uint8*)malloc(DRMTL_CHUNK_SIZE);
                if (pMaterial->pRawData != NULL)
                {
                    memcpy(pMaterial->pRawData, pRawData, dataSizeInBytes);
                    pMaterial->sizeInBytes          = dataSizeInBytes;
                    pMaterial->bufferSizeInBytes    = dataSizeInBytes;
                    pMaterial->currentStage         = DRMTL_STAGE_COMPLETE;
                    pMaterial->currentChannelOffset = 0;
                    pMaterial->ownsRawData          = 1;

                    return 1;
                }
            }
        }
    }

    return 0;
}

bool drmtl_initfromexisting_nocopy(drmtl_material* pMaterial, const void* pRawData, unsigned int dataSizeInBytes)
{
    if (pMaterial != NULL)
    {
        if (pRawData != NULL && dataSizeInBytes >= sizeof(drmtl_header))
        {
            if (((const drmtl_header*)pRawData)->magic == DRMTL_MAGIC_NUMBER)
            {
                pMaterial->pRawData             = (drmtl_uint8*)pRawData;
                pMaterial->sizeInBytes          = dataSizeInBytes;
                pMaterial->bufferSizeInBytes    = dataSizeInBytes;
                pMaterial->currentStage         = DRMTL_STAGE_COMPLETE;
                pMaterial->currentChannelOffset = 0;
                pMaterial->ownsRawData          = 0;

                return 1;
            }
        }
    }

    return 0;
}

void drmtl_uninit(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        if (pMaterial->ownsRawData)
        {
            free(pMaterial->pRawData);
        }

        pMaterial->pRawData = NULL;
    }
}


drmtl_header* drmtl_getheader(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        return (drmtl_header*)pMaterial->pRawData;
    }

    return NULL;
}


bool drmtl_appendidentifier(drmtl_material* pMaterial, drmtl_identifier identifier, unsigned int* indexOut)
{
    if (pMaterial != NULL)
    {
        if (pMaterial->currentStage <= DRMTL_STAGE_IDS)
        {
            drmtl_header* pHeader = drmtl_getheader(pMaterial);
            if (pHeader != NULL)
            {
                if (pMaterial->sizeInBytes + pHeader->identifierSizeInBytes > pMaterial->bufferSizeInBytes)
                {
                    if (!_drmtl_inflate(pMaterial))
                    {
                        // An error occured when trying to inflate the buffer. Might be out of memory.
                        return 0;
                    }

                    pHeader = drmtl_getheader(pMaterial);
                    assert(pMaterial->sizeInBytes + pHeader->identifierSizeInBytes <= pMaterial->bufferSizeInBytes);
                }


                memcpy(pMaterial->pRawData + pHeader->inputsOffset, &identifier, pHeader->identifierSizeInBytes);
                pMaterial->sizeInBytes += pHeader->identifierSizeInBytes;

                pHeader->identifierCount  += 1;
                pHeader->inputsOffset     += pHeader->identifierSizeInBytes;
                pHeader->channelsOffset   += pHeader->identifierSizeInBytes;
                pHeader->propertiesOffset += pHeader->identifierSizeInBytes;


                if (indexOut != NULL)
                {
                    *indexOut = pHeader->identifierCount - 1;
                }

                return 1;
            }
        }
    }

    return 0;
}

bool drmtl_appendprivateinput(drmtl_material* pMaterial, drmtl_input input)
{
    if (pMaterial != NULL)
    {
        if (pMaterial->currentStage <= DRMTL_STAGE_PRIVATE_INPUTS)
        {
            drmtl_header* pHeader = drmtl_getheader(pMaterial);
            if (pHeader != NULL)
            {
                if (pMaterial->sizeInBytes + pHeader->inputSizeInBytes > pMaterial->bufferSizeInBytes)
                {
                    if (!_drmtl_inflate(pMaterial))
                    {
                        // An error occured when trying to inflate the buffer. Might be out of memory.
                        return 0;
                    }

                    pHeader = drmtl_getheader(pMaterial);
                    assert(pMaterial->sizeInBytes + pHeader->inputSizeInBytes <= pMaterial->bufferSizeInBytes);
                }


                memcpy(pMaterial->pRawData + pHeader->channelsOffset, &input, pHeader->inputSizeInBytes);
                pMaterial->sizeInBytes += pHeader->inputSizeInBytes;

                pHeader->privateInputCount += 1;
                pHeader->channelsOffset    += pHeader->inputSizeInBytes;
                pHeader->propertiesOffset  += pHeader->inputSizeInBytes;


                pMaterial->currentStage = DRMTL_STAGE_PRIVATE_INPUTS;
                return 1;
            }
        }
    }

    return 0;
}

bool drmtl_appendpublicinput(drmtl_material* pMaterial, drmtl_input input)
{
    if (pMaterial != NULL)
    {
        if (pMaterial->currentStage <= DRMTL_STAGE_PUBLIC_INPUTS)
        {
            drmtl_header* pHeader = drmtl_getheader(pMaterial);
            if (pHeader != NULL)
            {
                if (pMaterial->sizeInBytes + pHeader->inputSizeInBytes > pMaterial->bufferSizeInBytes)
                {
                    if (!_drmtl_inflate(pMaterial))
                    {
                        // An error occured when trying to inflate the buffer. Might be out of memory.
                        return 0;
                    }

                    pHeader = drmtl_getheader(pMaterial);
                    assert(pMaterial->sizeInBytes + pHeader->inputSizeInBytes <= pMaterial->bufferSizeInBytes);
                }


                memcpy(pMaterial->pRawData + pHeader->channelsOffset, &input, pHeader->inputSizeInBytes);
                pMaterial->sizeInBytes += pHeader->inputSizeInBytes;

                pHeader->publicInputCount  += 1;
                pHeader->channelsOffset    += pHeader->inputSizeInBytes;
                pHeader->propertiesOffset  += pHeader->inputSizeInBytes;


                pMaterial->currentStage = DRMTL_STAGE_PUBLIC_INPUTS;
                return 1;
            }
        }
    }

    return 0;
}

bool drmtl_appendchannel(drmtl_material* pMaterial, drmtl_channel channel)
{
    if (pMaterial != NULL)
    {
        if (pMaterial->currentStage <= DRMTL_STAGE_CHANNELS)
        {
            drmtl_header* pHeader = drmtl_getheader(pMaterial);
            if (pHeader != NULL)
            {
                drmtl_channel_header channelHeader;
                channelHeader.channel         = channel;
                channelHeader.instructionCount = 0;

                if (pMaterial->sizeInBytes + pHeader->channelHeaderSizeInBytes > pMaterial->bufferSizeInBytes)
                {
                    if (!_drmtl_inflate(pMaterial))
                    {
                        // An error occured when trying to inflate the buffer. Might be out of memory.
                        return 0;
                    }

                    pHeader = drmtl_getheader(pMaterial);
                    assert(pMaterial->sizeInBytes + pHeader->channelHeaderSizeInBytes <= pMaterial->bufferSizeInBytes);
                }


                memcpy(pMaterial->pRawData + pHeader->propertiesOffset, &channelHeader, pHeader->channelHeaderSizeInBytes);
                pMaterial->currentChannelOffset = pMaterial->sizeInBytes;
                pMaterial->sizeInBytes += pHeader->channelHeaderSizeInBytes;

                pHeader->channelCount     += 1;
                pHeader->propertiesOffset += pHeader->channelHeaderSizeInBytes;


                pMaterial->currentStage = DRMTL_STAGE_CHANNELS;
                return 1;
            }
        }
    }

    return 0;
}

bool drmtl_appendinstruction(drmtl_material* pMaterial, drmtl_instruction instruction)
{
    if (pMaterial != NULL)
    {
        if (pMaterial->currentStage == DRMTL_STAGE_CHANNELS)
        {
            drmtl_header* pHeader = drmtl_getheader(pMaterial);
            if (pHeader != NULL)
            {
                if (pMaterial->sizeInBytes + pHeader->instructionSizeInBytes > pMaterial->bufferSizeInBytes)
                {
                    if (!_drmtl_inflate(pMaterial))
                    {
                        // An error occured when trying to inflate the buffer. Might be out of memory.
                        return 0;
                    }

                    pHeader = drmtl_getheader(pMaterial);
                    assert(pMaterial->sizeInBytes + pHeader->instructionSizeInBytes <= pMaterial->bufferSizeInBytes);
                }


                memcpy(pMaterial->pRawData + pHeader->propertiesOffset, &instruction, pHeader->instructionSizeInBytes);
                pMaterial->sizeInBytes += pHeader->instructionSizeInBytes;

                drmtl_channel_header* pChannelHeader = (drmtl_channel_header*)(pMaterial->pRawData + pMaterial->currentChannelOffset);
                if (pChannelHeader != NULL)
                {
                    pChannelHeader->instructionCount += 1;
                }

                pHeader->propertiesOffset += pHeader->instructionSizeInBytes;


                return 1;
            }
        }
    }

    return 0;
}

bool drmtl_appendproperty(drmtl_material* pMaterial, drmtl_property prop)
{
    if (pMaterial != NULL)
    {
        if (pMaterial->currentStage <= DRMTL_STAGE_CHANNELS)
        {
            drmtl_header* pHeader = drmtl_getheader(pMaterial);
            if (pHeader != NULL)
            {
                if (pMaterial->sizeInBytes + pHeader->propertySizeInBytes > pMaterial->bufferSizeInBytes)
                {
                    if (!_drmtl_inflate(pMaterial))
                    {
                        // An error occured when trying to inflate the buffer. Might be out of memory.
                        return 0;
                    }

                    pHeader = drmtl_getheader(pMaterial);
                    assert(pMaterial->sizeInBytes + pHeader->propertySizeInBytes <= pMaterial->bufferSizeInBytes);
                }


                memcpy(pMaterial->pRawData + pMaterial->sizeInBytes, &prop, pHeader->propertySizeInBytes);
                pMaterial->sizeInBytes += pHeader->propertySizeInBytes;

                pHeader->propertyCount += 1;


                pMaterial->currentStage = DRMTL_STAGE_PROPERTIES;
                return 1;
            }
        }
    }

    return 0;
}


drmtl_channel_header* drmtl_getchannelheaderbyindex(drmtl_material* pMaterial, unsigned int channelIndex)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        if (channelIndex < pHeader->channelCount)
        {
            drmtl_uint8* pChannelHeader = pMaterial->pRawData + pHeader->channelsOffset;
            for (unsigned int iChannel = 0; iChannel < channelIndex; ++iChannel)
            {
                pChannelHeader += sizeof(drmtl_channel_header) + (sizeof(drmtl_instruction) * ((drmtl_channel_header*)pChannelHeader)->instructionCount);
            }

            return (drmtl_channel_header*)pChannelHeader;
        }
    }

    return NULL;
}

drmtl_channel_header* drmtl_getchannelheaderbyname(drmtl_material* pMaterial, const char* channelName)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        drmtl_uint8* pChannelHeader = pMaterial->pRawData + pHeader->channelsOffset;
        for (unsigned int iChannel = 0; iChannel < pHeader->channelCount; ++iChannel)
        {
            if (strcmp(((drmtl_channel_header*)pChannelHeader)->channel.name, channelName) == 0)
            {
                return (drmtl_channel_header*)pChannelHeader;
            }

            pChannelHeader += sizeof(drmtl_channel_header) + (sizeof(drmtl_instruction) * ((drmtl_channel_header*)pChannelHeader)->instructionCount);
        }
    }

    return NULL;
}

drmtl_identifier* drmtl_getidentifiers(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        return (drmtl_identifier*)(pMaterial->pRawData + pHeader->identifiersOffset);
    }

    return NULL;
}

drmtl_identifier* drmtl_getidentifier(drmtl_material* pMaterial, unsigned int index)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        if (index < pHeader->identifierCount)
        {
            drmtl_identifier* firstIdentifier = (drmtl_identifier*)(pMaterial->pRawData + pHeader->identifiersOffset);
            return firstIdentifier + index;
        }
    }

    return NULL;
}

unsigned int drmtl_getidentifiercount(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        return pHeader->identifierCount;
    }

    return 0;
}


unsigned int drmtl_getinputcount(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        return pHeader->privateInputCount + pHeader->publicInputCount;
    }

    return 0;
}

drmtl_input* drmtl_getinputbyindex(drmtl_material* pMaterial, unsigned int index)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        if (index < (pHeader->privateInputCount + pHeader->publicInputCount))
        {
            drmtl_input* firstInput = (drmtl_input*)(pMaterial->pRawData + pHeader->inputsOffset);
            return firstInput + index;
        }
    }

    return NULL;
}

unsigned int drmtl_getprivateinputcount(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        return pHeader->privateInputCount;
    }

    return 0;
}

drmtl_input* drmtl_getprivateinputbyindex(drmtl_material* pMaterial, unsigned int index)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        if (index < pHeader->privateInputCount)
        {
            drmtl_input* firstInput = (drmtl_input*)(pMaterial->pRawData + pHeader->inputsOffset);
            return firstInput + index;
        }
    }

    return NULL;
}

unsigned int drmtl_getpublicinputcount(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        return pHeader->publicInputCount;
    }

    return 0;
}

drmtl_input* drmtl_getpublicinputbyindex(drmtl_material* pMaterial, unsigned int index)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        if (index < pHeader->publicInputCount)
        {
            drmtl_input* firstInput = (drmtl_input*)(pMaterial->pRawData + pHeader->inputsOffset);
            return firstInput + pHeader->privateInputCount + index;
        }
    }

    return NULL;
}


unsigned int drmtl_getpropertycount(drmtl_material* pMaterial)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        return pHeader->propertyCount;
    }

    return 0;
}

drmtl_property* drmtl_getpropertybyindex(drmtl_material* pMaterial, unsigned int index)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        if (index < pHeader->propertyCount)
        {
            drmtl_property* firstProperty = (drmtl_property*)(pMaterial->pRawData + pHeader->propertiesOffset);
            return firstProperty + index;
        }
    }

    return NULL;
}

drmtl_property* drmtl_getpropertybyname(drmtl_material* pMaterial, const char* name)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        assert(pHeader != NULL);

        for (unsigned int i = 0; i < pHeader->propertyCount; ++i)
        {
            drmtl_property* pProperty = ((drmtl_property*)(pMaterial->pRawData + pHeader->propertiesOffset)) + i;
            assert(pProperty != NULL);

            if (strcmp(pProperty->name, name) == 0)
            {
                return pProperty;
            }
        }
    }

    return NULL;
}


//////////////////////////////////
// Private low-level API.

bool _drmtl_inflate(drmtl_material* pMaterial)
{
    assert(pMaterial != NULL);

    drmtl_uint8* pOldBuffer = pMaterial->pRawData;
    drmtl_uint8* pNewBuffer = (drmtl_uint8*)malloc(pMaterial->bufferSizeInBytes + DRMTL_CHUNK_SIZE);
    if (pNewBuffer != NULL)
    {
        memcpy(pNewBuffer, pOldBuffer, pMaterial->sizeInBytes);
        pMaterial->pRawData = pNewBuffer;
        pMaterial->bufferSizeInBytes += DRMTL_CHUNK_SIZE;

        free(pOldBuffer);
        return 1;
    }

    return 0;
}



////////////////////////////////////////////////////////
// Mid-Level APIs

drmtl_identifier drmtl_identifier_float(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_float;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_float2(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_float2;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_float3(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_float3;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_float4(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_float4;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_int(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_int;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_int2(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_int2;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_int3(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_int3;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_int4(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_int4;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}

drmtl_identifier drmtl_identifier_tex2d(const char* name)
{
    drmtl_identifier identifier;
    identifier.type = drmtl_type_tex2d;
    drmtl_strcpy(identifier.name, DRMTL_MAX_IDENTIFIER_NAME, name);

    return identifier;
}


drmtl_input drmtl_input_float(unsigned int identifierIndex, float x)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.f1.x = x;

    return input;
}

drmtl_input drmtl_input_float2(unsigned int identifierIndex, float x, float y)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.f2.x = x;
    input.f2.y = y;

    return input;
}

drmtl_input drmtl_input_float3(unsigned int identifierIndex, float x, float y, float z)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.f3.x = x;
    input.f3.y = y;
    input.f3.z = z;

    return input;
}

drmtl_input drmtl_input_float4(unsigned int identifierIndex, float x, float y, float z, float w)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.f4.x = x;
    input.f4.y = y;
    input.f4.z = z;
    input.f4.w = w;

    return input;
}

drmtl_input drmtl_input_int(unsigned int identifierIndex, int x)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.i1.x = x;

    return input;
}

drmtl_input drmtl_input_int2(unsigned int identifierIndex, int x, int y)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.i2.x = x;
    input.i2.y = y;

    return input;
}

drmtl_input drmtl_input_int3(unsigned int identifierIndex, int x, int y, int z)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.i3.x = x;
    input.i3.y = y;
    input.i3.z = z;

    return input;
}

drmtl_input drmtl_input_int4(unsigned int identifierIndex, int x, int y, int z, int w)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    input.i4.x = x;
    input.i4.y = y;
    input.i4.z = z;
    input.i4.w = w;

    return input;
}

drmtl_input drmtl_input_tex(unsigned int identifierIndex, const char* path)
{
    drmtl_input input;
    input.identifierIndex = identifierIndex;
    drmtl_strcpy(input.path.value, DRMTL_MAX_INPUT_PATH, path);

    return input;
}


drmtl_channel drmtl_channel_float(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_float;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}

drmtl_channel drmtl_channel_float2(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_float2;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}

drmtl_channel drmtl_channel_float3(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_float3;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}

drmtl_channel drmtl_channel_float4(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_float4;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}

drmtl_channel drmtl_channel_int(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_int;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}

drmtl_channel drmtl_channel_int2(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_int2;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}

drmtl_channel drmtl_channel_int3(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_int3;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}

drmtl_channel drmtl_channel_int4(const char* name)
{
    drmtl_channel channel;
    channel.type = drmtl_type_int4;
    drmtl_strcpy(channel.name, DRMTL_MAX_CHANNEL_NAME, name);

    return channel;
}


drmtl_instruction drmtl_movf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf1;
    inst.mov.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.mov.inputX.id   = inputIdentifierIndex;
    inst.mov.output      = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_movf1_c1(unsigned int outputIdentifierIndex, float x)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf1;
    inst.mov.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputX.valuef = x;
    inst.mov.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_movf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf2;
    inst.mov.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.mov.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.mov.inputX.id   = inputIdentifierIndex;
    inst.mov.inputY.id   = inputIdentifierIndex;
    inst.mov.output      = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_movf2_c2(unsigned int outputIdentifierIndex, float x, float y)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf2;
    inst.mov.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputX.valuef = x;
    inst.mov.inputY.valuef = y;
    inst.mov.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_movf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf3;
    inst.mov.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.mov.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.mov.inputDesc.z = DRMTL_INPUT_DESC_VARZ;
    inst.mov.inputX.id   = inputIdentifierIndex;
    inst.mov.inputY.id   = inputIdentifierIndex;
    inst.mov.inputZ.id   = inputIdentifierIndex;
    inst.mov.output      = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_movf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf3;
    inst.mov.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputX.valuef = x;
    inst.mov.inputY.valuef = y;
    inst.mov.inputZ.valuef = z;
    inst.mov.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_movf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf4;
    inst.mov.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.mov.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.mov.inputDesc.z = DRMTL_INPUT_DESC_VARZ;
    inst.mov.inputDesc.w = DRMTL_INPUT_DESC_VARW;
    inst.mov.inputX.id   = inputIdentifierIndex;
    inst.mov.inputY.id   = inputIdentifierIndex;
    inst.mov.inputZ.id   = inputIdentifierIndex;
    inst.mov.inputW.id   = inputIdentifierIndex;
    inst.mov.output      = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_movf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_movf4;
    inst.mov.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.mov.inputX.valuef = x;
    inst.mov.inputY.valuef = y;
    inst.mov.inputZ.valuef = z;
    inst.mov.inputW.valuef = w;
    inst.mov.output        = outputIdentifierIndex;

    return inst;
}


drmtl_instruction drmtl_addf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf1;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.add.inputX.id     = inputIdentifierIndex;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_addf1_c1(unsigned int outputIdentifierIndex, float x)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf1;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputX.valuef = x;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_addf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf2;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.add.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.add.inputX.id     = inputIdentifierIndex;
    inst.add.inputY.id     = inputIdentifierIndex;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_addf2_c2(unsigned int outputIdentifierIndex, float x, float y)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf2;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputX.valuef = x;
    inst.add.inputY.valuef = y;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_addf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf3;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.add.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.add.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.add.inputX.id     = inputIdentifierIndex;
    inst.add.inputY.id     = inputIdentifierIndex;
    inst.add.inputZ.id     = inputIdentifierIndex;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_addf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf3;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputX.valuef = x;
    inst.add.inputY.valuef = y;
    inst.add.inputZ.valuef = z;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_addf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf4;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.add.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.add.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.add.inputDesc.w   = DRMTL_INPUT_DESC_VARW;
    inst.add.inputX.id     = inputIdentifierIndex;
    inst.add.inputY.id     = inputIdentifierIndex;
    inst.add.inputZ.id     = inputIdentifierIndex;
    inst.add.inputW.id     = inputIdentifierIndex;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_addf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_addf4;
    inst.add.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.add.inputX.valuef = x;
    inst.add.inputY.valuef = y;
    inst.add.inputZ.valuef = z;
    inst.add.inputW.valuef = w;
    inst.add.output        = outputIdentifierIndex;

    return inst;
}


drmtl_instruction drmtl_subf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf1;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.sub.inputX.id     = inputIdentifierIndex;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_subf1_c1(unsigned int outputIdentifierIndex, float x)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf1;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputX.valuef = x;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_subf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf2;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.sub.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.sub.inputX.id     = inputIdentifierIndex;
    inst.sub.inputY.id     = inputIdentifierIndex;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_subf2_c2(unsigned int outputIdentifierIndex, float x, float y)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf2;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputX.valuef = x;
    inst.sub.inputY.valuef = y;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_subf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf3;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.sub.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.sub.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.sub.inputX.id     = inputIdentifierIndex;
    inst.sub.inputY.id     = inputIdentifierIndex;
    inst.sub.inputZ.id     = inputIdentifierIndex;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_subf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf3;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputX.valuef = x;
    inst.sub.inputY.valuef = y;
    inst.sub.inputZ.valuef = z;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_subf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf4;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.sub.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.sub.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.sub.inputDesc.w   = DRMTL_INPUT_DESC_VARW;
    inst.sub.inputX.id     = inputIdentifierIndex;
    inst.sub.inputY.id     = inputIdentifierIndex;
    inst.sub.inputZ.id     = inputIdentifierIndex;
    inst.sub.inputW.id     = inputIdentifierIndex;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_subf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_subf4;
    inst.sub.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.sub.inputX.valuef = x;
    inst.sub.inputY.valuef = y;
    inst.sub.inputZ.valuef = z;
    inst.sub.inputW.valuef = w;
    inst.sub.output        = outputIdentifierIndex;

    return inst;
}


drmtl_instruction drmtl_mulf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf1;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputX.id     = inputIdentifierIndex;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf1_c1(unsigned int outputIdentifierIndex, float x)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf1;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputX.valuef = x;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf2;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.mul.inputX.id     = inputIdentifierIndex;
    inst.mul.inputY.id     = inputIdentifierIndex;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf2_c2(unsigned int outputIdentifierIndex, float x, float y)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf2;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputX.valuef = x;
    inst.mul.inputY.valuef = y;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf3;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.mul.inputX.id     = inputIdentifierIndex;
    inst.mul.inputY.id     = inputIdentifierIndex;
    inst.mul.inputZ.id     = inputIdentifierIndex;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf3;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputX.valuef = x;
    inst.mul.inputY.valuef = y;
    inst.mul.inputZ.valuef = z;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf4;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.mul.inputDesc.w   = DRMTL_INPUT_DESC_VARW;
    inst.mul.inputX.id     = inputIdentifierIndex;
    inst.mul.inputY.id     = inputIdentifierIndex;
    inst.mul.inputZ.id     = inputIdentifierIndex;
    inst.mul.inputW.id     = inputIdentifierIndex;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf4;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputX.valuef = x;
    inst.mul.inputY.valuef = y;
    inst.mul.inputZ.valuef = z;
    inst.mul.inputW.valuef = w;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf4_v3v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndexXYZ, unsigned int inputIdentifierIndexW)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf4;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.mul.inputDesc.w   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputX.id     = inputIdentifierIndexXYZ;
    inst.mul.inputY.id     = inputIdentifierIndexXYZ;
    inst.mul.inputZ.id     = inputIdentifierIndexXYZ;
    inst.mul.inputW.id     = inputIdentifierIndexW;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf4_v3c1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf4;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.mul.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputX.id     = inputIdentifierIndex;
    inst.mul.inputY.id     = inputIdentifierIndex;
    inst.mul.inputZ.id     = inputIdentifierIndex;
    inst.mul.inputW.valuef = w;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf4_v2c2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf4;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputX.id     = inputIdentifierIndex;
    inst.mul.inputY.id     = inputIdentifierIndex;
    inst.mul.inputZ.valuef = z;
    inst.mul.inputW.valuef = w;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf4_v1c3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex, float y, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf4;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.mul.inputX.id     = inputIdentifierIndex;
    inst.mul.inputY.valuef = y;
    inst.mul.inputZ.valuef = z;
    inst.mul.inputW.valuef = w;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_mulf4_v1v1v1v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndexX, unsigned int inputIdentifierIndexY, unsigned int inputIdentifierIndexZ, unsigned int inputIdentifierIndexW)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_mulf4;
    inst.mul.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.y   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.z   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputDesc.w   = DRMTL_INPUT_DESC_VARX;
    inst.mul.inputX.id     = inputIdentifierIndexX;
    inst.mul.inputY.id     = inputIdentifierIndexY;
    inst.mul.inputZ.id     = inputIdentifierIndexZ;
    inst.mul.inputW.id     = inputIdentifierIndexW;
    inst.mul.output        = outputIdentifierIndex;

    return inst;
}


drmtl_instruction drmtl_divf1_v1(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf1;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.div.inputX.id     = inputIdentifierIndex;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_divf1_c1(unsigned int outputIdentifierIndex, float x)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf1;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputX.valuef = x;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_divf2_v2(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf2;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.div.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.div.inputX.id     = inputIdentifierIndex;
    inst.div.inputY.id     = inputIdentifierIndex;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_divf2_c2(unsigned int outputIdentifierIndex, float x, float y)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf2;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputX.valuef = x;
    inst.div.inputY.valuef = y;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_divf3_v3(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf3;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.div.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.div.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.div.inputX.id     = inputIdentifierIndex;
    inst.div.inputY.id     = inputIdentifierIndex;
    inst.div.inputZ.id     = inputIdentifierIndex;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_divf3_c3(unsigned int outputIdentifierIndex, float x, float y, float z)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf3;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputX.valuef = x;
    inst.div.inputY.valuef = y;
    inst.div.inputZ.valuef = z;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_divf4_v4(unsigned int outputIdentifierIndex, unsigned int inputIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf4;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_VARX;
    inst.div.inputDesc.y   = DRMTL_INPUT_DESC_VARY;
    inst.div.inputDesc.z   = DRMTL_INPUT_DESC_VARZ;
    inst.div.inputDesc.w   = DRMTL_INPUT_DESC_VARW;
    inst.div.inputX.id     = inputIdentifierIndex;
    inst.div.inputY.id     = inputIdentifierIndex;
    inst.div.inputZ.id     = inputIdentifierIndex;
    inst.div.inputW.id     = inputIdentifierIndex;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_divf4_c4(unsigned int outputIdentifierIndex, float x, float y, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_divf4;
    inst.div.inputDesc.x   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputDesc.y   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputDesc.z   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputDesc.w   = DRMTL_INPUT_DESC_CONSTF;
    inst.div.inputX.valuef = x;
    inst.div.inputY.valuef = y;
    inst.div.inputZ.valuef = z;
    inst.div.inputW.valuef = w;
    inst.div.output        = outputIdentifierIndex;

    return inst;
}


drmtl_instruction drmtl_tex2(unsigned int outputIdentifierIndex, unsigned int textureIdentifierIndex, unsigned int texcoordIdentifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_tex2;
    inst.tex.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.tex.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.tex.inputX.id   = texcoordIdentifierIndex;
    inst.tex.inputY.id   = texcoordIdentifierIndex;
    inst.tex.texture     = textureIdentifierIndex;
    inst.tex.output      = outputIdentifierIndex;

    return inst;
}

drmtl_instruction drmtl_var(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_var;
    inst.var.identifierIndex = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_retf1(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf1;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputX.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_retf2(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf2;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.ret.inputX.id = identifierIndex;
    inst.ret.inputY.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_retf3(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf3;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_VARZ;
    inst.ret.inputX.id = identifierIndex;
    inst.ret.inputY.id = identifierIndex;
    inst.ret.inputZ.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_retf4(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf4;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_VARZ;
    inst.ret.inputDesc.w = DRMTL_INPUT_DESC_VARW;
    inst.ret.inputX.id = identifierIndex;
    inst.ret.inputY.id = identifierIndex;
    inst.ret.inputZ.id = identifierIndex;
    inst.ret.inputW.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_retf1_c1(float x)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf1;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputX.valuef = x;

    return inst;
}

drmtl_instruction drmtl_retf2_c2(float x, float y)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf2;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputX.valuef = x;
    inst.ret.inputY.valuef = y;

    return inst;
}

drmtl_instruction drmtl_retf3_c3(float x, float y, float z)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf3;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputX.valuef = x;
    inst.ret.inputY.valuef = y;
    inst.ret.inputZ.valuef = z;

    return inst;
}

drmtl_instruction drmtl_retf4_c4(float x, float y, float z, float w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_retf4;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputDesc.w = DRMTL_INPUT_DESC_CONSTF;
    inst.ret.inputX.valuef = x;
    inst.ret.inputY.valuef = y;
    inst.ret.inputZ.valuef = z;
    inst.ret.inputW.valuef = w;

    return inst;
}

drmtl_instruction drmtl_reti1(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti1;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputX.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_reti2(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti2;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.ret.inputX.id = identifierIndex;
    inst.ret.inputY.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_reti3(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti3;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_VARZ;
    inst.ret.inputX.id = identifierIndex;
    inst.ret.inputY.id = identifierIndex;
    inst.ret.inputZ.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_reti4(unsigned int identifierIndex)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti4;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_VARX;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_VARY;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_VARZ;
    inst.ret.inputDesc.w = DRMTL_INPUT_DESC_VARW;
    inst.ret.inputX.id = identifierIndex;
    inst.ret.inputY.id = identifierIndex;
    inst.ret.inputZ.id = identifierIndex;
    inst.ret.inputW.id = identifierIndex;

    return inst;
}

drmtl_instruction drmtl_reti1_c1(int x)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti1;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputX.valuei = x;

    return inst;
}

drmtl_instruction drmtl_reti2_c2(int x, int y)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti2;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputX.valuei = x;
    inst.ret.inputY.valuei = y;

    return inst;
}

drmtl_instruction drmtl_reti3_c3(int x, int y, int z)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti3;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputX.valuei = x;
    inst.ret.inputY.valuei = y;
    inst.ret.inputZ.valuei = z;

    return inst;
}

drmtl_instruction drmtl_reti4_c4(int x, int y, int z, int w)
{
    drmtl_instruction inst;
    inst.opcode = drmtl_opcode_reti4;
    inst.ret.inputDesc.x = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputDesc.y = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputDesc.z = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputDesc.w = DRMTL_INPUT_DESC_CONSTI;
    inst.ret.inputX.valuei = x;
    inst.ret.inputY.valuei = y;
    inst.ret.inputZ.valuei = z;
    inst.ret.inputW.valuei = w;

    return inst;
}



drmtl_property drmtl_property_float(const char* name, float x)
{
    drmtl_property prop;
    prop.type = drmtl_type_float;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.f1.x = x;

    return prop;
}

drmtl_property drmtl_property_float2(const char* name, float x, float y)
{
    drmtl_property prop;
    prop.type = drmtl_type_float2;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.f2.x = x;
    prop.f2.y = y;

    return prop;
}

drmtl_property drmtl_property_float3(const char* name, float x, float y, float z)
{
    drmtl_property prop;
    prop.type = drmtl_type_float3;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.f3.x = x;
    prop.f3.y = y;
    prop.f3.z = z;

    return prop;
}

drmtl_property drmtl_property_float4(const char* name, float x, float y, float z, float w)
{
    drmtl_property prop;
    prop.type = drmtl_type_float4;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.f4.x = x;
    prop.f4.y = y;
    prop.f4.z = z;
    prop.f4.w = w;

    return prop;
}

drmtl_property drmtl_property_int(const char* name, int x)
{
    drmtl_property prop;
    prop.type = drmtl_type_int;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.i1.x = x;

    return prop;
}

drmtl_property drmtl_property_int2(const char* name, int x, int y)
{
    drmtl_property prop;
    prop.type = drmtl_type_int2;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.i2.x = x;
    prop.i2.y = y;

    return prop;
}

drmtl_property drmtl_property_int3(const char* name, int x, int y, int z)
{
    drmtl_property prop;
    prop.type = drmtl_type_int3;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.i3.x = x;
    prop.i3.y = y;
    prop.i3.z = z;

    return prop;
}

drmtl_property drmtl_property_int4(const char* name, int x, int y, int z, int w)
{
    drmtl_property prop;
    prop.type = drmtl_type_int4;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.i4.x = x;
    prop.i4.y = y;
    prop.i4.z = z;
    prop.i4.w = w;

    return prop;
}

drmtl_property drmtl_property_bool(const char* name, bool value)
{
    drmtl_property prop;
    prop.type = drmtl_type_bool;
    drmtl_strcpy(prop.name, DRMTL_MAX_PROPERTY_NAME, name);
    prop.b1.x = value;

    return prop;
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Compilers
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef DRMTL_NO_MTL_COMPILER
typedef struct
{
    /// A pointer to the buffer containing the Wavefront MTL data.
    const char* pData;

    /// The size of the data buffer size.
    unsigned int dataSizeInBytes;

    /// A pointer to the next bytes to read.
    const char* pDataCur;

    /// A pointer to the end of the buffer.
    const char* pDataEnd;


    /// The diffuse colour.
    float diffuse[3];

    /// The diffuse map.
    char diffuseMap[DRMTL_MAX_INPUT_PATH];


    /// The specular colour.
    float specular[3];

    /// The specular map.
    char specularMap[DRMTL_MAX_INPUT_PATH];


    /// The specular exponent.
    float specularExponent;

    /// The specular exponent map.
    char specularExponentMap[DRMTL_MAX_INPUT_PATH];


    /// The alpha transparency value.
    float alpha;

    /// The alpha transparency map.
    char alphaMap[DRMTL_MAX_INPUT_PATH];


} drmtl_wavefront;

bool drmtl_wavefront_is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

bool drmtl_wavefront_is_valid_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool drmtl_wavefront_atof(const char* str, const char* strEnd, const char** strEndOut, float* valueOut)
{
    // Skip leading whitespace.
    while (str < strEnd && drmtl_wavefront_is_whitespace(*str))
    {
        str += 1;
    }


    // Check that we have a string after moving the whitespace.
    if (str < strEnd)
    {
        float sign  = 1.0f;
        float value = 0.0f;

        // Sign.
        if (*str == '-')
        {
            sign = -1.0f;
            str += 1;
        }
        else if (*str == '+')
        {
            sign = 1.0f;
            str += 1;
        }


        // Digits before the decimal point.
        while (str < strEnd && drmtl_wavefront_is_valid_digit(*str))
        {
            value = value * 10.0f + (*str - '0');

            str += 1;
        }

        // Digits after the decimal point.
        if (*str == '.')
        {
            float pow10 = 10.0f;

            str += 1;
            while (str < strEnd && drmtl_wavefront_is_valid_digit(*str))
            {
                value += (*str - '0') / pow10;
                pow10 *= 10.0f;

                str += 1;
            }
        }


        if (strEndOut != NULL)
        {
            *strEndOut = str;
        }

        if (valueOut != NULL)
        {
            *valueOut = sign * value;
        }

        return 1;
    }
    else
    {
        // Empty string. Leave output untouched and return 0.
        return 0;
    }
}

bool drmtl_wavefront_atof_3(const char* str, const char* strEnd, const char** strEndOut, float valueOut[3])
{
    float value[3];
    if (drmtl_wavefront_atof(str, strEnd, &str, &value[0]))
    {
        value[1] = value[0];
        value[2] = value[0];

        if (drmtl_wavefront_atof(str, strEnd, &str, &value[1]))
        {
            // We got two numbers which means we must have the third for this to be successful.
            if (!drmtl_wavefront_atof(str, strEnd, strEndOut, &value[2]))
            {
                // Failed to get the third number. We only found 2 which is not valid. Error.
                return 0;
            }
        }


        valueOut[0] = value[0];
        valueOut[1] = value[1];
        valueOut[2] = value[2];

        return 1;
    }

    return 0;
}

const char* drmtl_wavefront_find_end_of_line(const char* pDataCur, const char* pDataEnd)
{
    assert(pDataCur != NULL);
    assert(pDataEnd != NULL);

    while (pDataCur < pDataEnd)
    {
        if (pDataCur[0] == '\n')
        {
            return pDataCur;
        }
        else
        {
            if (pDataCur + 1 < pDataEnd)
            {
                if (pDataCur[0] == '\r' && pDataCur[1] == '\n')
                {
                    return pDataCur;
                }
            }
        }

        pDataCur += 1;
    }

    // If we get here it means we hit the end of the file before find a new-line character.
    return pDataEnd;
}

const char* drmtl_wavefront_find_next_line(const char* pDataCur, const char* pDataEnd)
{
    assert(pDataCur != NULL);
    assert(pDataEnd != NULL);

    pDataCur = drmtl_wavefront_find_end_of_line(pDataCur, pDataEnd);
    if (pDataCur != NULL)
    {
        if (pDataCur < pDataEnd)
        {
            if (pDataCur[0] == '\n')
            {
                return pDataCur + 1;
            }
            else
            {
                if (pDataCur + 1 < pDataEnd)
                {
                    if (pDataCur[0] == '\r' && pDataCur[1] == '\n')
                    {
                        return pDataCur + 2;
                    }
                }
            }
        }
    }

    return NULL;
}

const char* drmtl_wavefront_find_next_newmtl(const char* pDataCur, const char* pDataEnd)
{
    assert(pDataCur != NULL);
    assert(pDataEnd != NULL);

    while (pDataCur + 7 < pDataEnd)   // +7 for "newmtl" + a whitespace character.
    {
        if (pDataCur[0] == 'n' && pDataCur[1] == 'e' && pDataCur[2] == 'w' && pDataCur[3] == 'm' && pDataCur[4] == 't' && pDataCur[5] == 'l')
        {
            // We found "newmtl", however the next line must be whitespace.
            if (drmtl_wavefront_is_whitespace(pDataCur[6]))
            {
                // We found it.
                return pDataCur;
            }
        }


        const char* nextLineStart = drmtl_wavefront_find_next_line(pDataCur, pDataEnd);
        if (nextLineStart != NULL)
        {
            pDataCur = nextLineStart;
        }
        else
        {
            // Reached the end before finding "newmtl". Return null.
            return NULL;
        }
    }

    return NULL;
}

const char* drmtl_wavefront_find_next_nonwhitespace(const char* pDataCur, const char* pDataEnd)
{
    assert(pDataCur != NULL);
    assert(pDataEnd != NULL);

    while (pDataCur < pDataEnd)
    {
        if (!drmtl_wavefront_is_whitespace(pDataCur[0]))
        {
            return pDataCur;
        }

        pDataCur += 1;
    }

    return NULL;
}


bool drmtl_wavefront_parse_K(const char* pDataCur, const char* pDataEnd, float valueOut[3])
{
    assert(pDataCur != NULL);
    assert(pDataEnd != NULL);

    return drmtl_wavefront_atof_3(pDataCur, pDataEnd, &pDataEnd, valueOut);
}

bool drmtl_wavefront_parse_N(const char* pDataCur, const char* pDataEnd, float* valueOut)
{
    assert(pDataCur != NULL);
    assert(pDataEnd != NULL);

    return drmtl_wavefront_atof(pDataCur, pDataEnd, &pDataEnd, valueOut);
}

bool drmtl_wavefront_parse_map(const char* pDataCur, const char* pDataEnd, char* pathOut, unsigned int pathSizeInBytes)
{
    assert(pDataCur != NULL);
    assert(pDataEnd != NULL);

    // For now we're not supporting options, however support for that will be added later.

    const char* pPathStart = drmtl_wavefront_find_next_nonwhitespace(pDataCur, pDataEnd);
    if (pPathStart != NULL)
    {
        if (pPathStart < pDataEnd)
        {
            if (pPathStart[0] != '#')
            {
                // Find the last non-whitespace, making sure we don't include comments.
                pDataCur = pPathStart;
                const char* pPathEnd = pDataCur;
                while (pDataCur < pDataEnd && pDataCur[0] != '#')
                {
                    if (!drmtl_wavefront_is_whitespace(pDataCur[0]))
                    {
                        pPathEnd = pDataCur + 1;
                    }

                    pDataCur += 1;
                }

                assert(pPathStart < pPathEnd);

                ptrdiff_t pathLength = pPathEnd - pPathStart;
                if ((size_t)pathLength + 1 < pathSizeInBytes)
                {
                    memcpy(pathOut, pPathStart, (size_t)pathLength);
                    pathOut[pathLength] = '\0';

                    return 1;
                }
            }
        }
    }

    return 0;
}


bool drmtl_wavefront_seek_to_next_line(drmtl_wavefront* pWavefront)
{
    assert(pWavefront != NULL);

    const char* lineStart = drmtl_wavefront_find_next_line(pWavefront->pDataCur, pWavefront->pDataEnd);
    if (lineStart != NULL)
    {
        pWavefront->pDataCur = lineStart;
        return 1;
    }

    return 0;
}

bool drmtl_wavefront_seek_to_newmtl(drmtl_wavefront* pWavefront)
{
    assert(pWavefront != NULL);

    const char* usemtl = drmtl_wavefront_find_next_newmtl(pWavefront->pDataCur, pWavefront->pDataEnd);
    if (usemtl != NULL)
    {
        pWavefront->pDataCur = usemtl;
        return 1;
    }

    return 0;
}

bool drmtl_wavefront_parse(drmtl_wavefront* pWavefront)
{
    assert(pWavefront != NULL);

    if (drmtl_wavefront_seek_to_newmtl(pWavefront) && drmtl_wavefront_seek_to_next_line(pWavefront))
    {
        // Set the end of the material to the start of the second usemtl statement, if it exists.
        const char* usemtl2 = drmtl_wavefront_find_next_newmtl(pWavefront->pDataCur, pWavefront->pDataEnd);
        if (usemtl2 != NULL)
        {
            pWavefront->pDataEnd = usemtl2;
        }


        while (pWavefront->pDataCur < pWavefront->pDataEnd)
        {
            const char* lineCur = pWavefront->pDataCur;
            const char* lineEnd = drmtl_wavefront_find_end_of_line(lineCur, pWavefront->pDataEnd);

            lineCur = drmtl_wavefront_find_next_nonwhitespace(lineCur, lineEnd);
            if (lineCur != NULL && (lineCur + 2 < lineEnd))
            {
                if (lineCur[0] == 'K' && lineCur[1] == 'd' && drmtl_wavefront_is_whitespace(lineCur[2]))          // Diffuse colour
                {
                    lineCur += 3;
                    drmtl_wavefront_parse_K(lineCur, lineEnd, pWavefront->diffuse);
                }
                else if (lineCur[0] == 'K' && lineCur[1] == 's' && drmtl_wavefront_is_whitespace(lineCur[2]))     // Specular colour
                {
                    lineCur += 3;
                    drmtl_wavefront_parse_K(lineCur, lineEnd, pWavefront->specular);
                }
                else if (lineCur[0] == 'N' && lineCur[1] == 's' && drmtl_wavefront_is_whitespace(lineCur[2]))     // Specular exponent
                {
                    lineCur += 3;
                    drmtl_wavefront_parse_N(lineCur, lineEnd, &pWavefront->specularExponent);
                }
                else if (lineCur[0] == 'd' && drmtl_wavefront_is_whitespace(lineCur[1]))                          // Opacity/Alpha
                {
                    lineCur += 2;
                    drmtl_wavefront_parse_N(lineCur, lineEnd, &pWavefront->alpha);
                }
                else
                {
                    // Check for maps.
                    if (lineCur + 6 < lineEnd)
                    {
                        if (lineCur[0] == 'm' && lineCur[1] == 'a' && lineCur[2] == 'p' && lineCur[3] == '_')
                        {
                            if (lineCur[4] == 'K' && lineCur[5] == 'd' && drmtl_wavefront_is_whitespace(lineCur[6]))          // Diffuse map
                            {
                                lineCur += 7;
                                drmtl_wavefront_parse_map(lineCur, lineEnd, pWavefront->diffuseMap, DRMTL_MAX_INPUT_PATH);
                            }
                            else if (lineCur[4] == 'K' && lineCur[5] == 's' && drmtl_wavefront_is_whitespace(lineCur[6]))     // Specular map
                            {
                                lineCur += 7;
                                drmtl_wavefront_parse_map(lineCur, lineEnd, pWavefront->specularMap, DRMTL_MAX_INPUT_PATH);
                            }
                            else if (lineCur[4] == 'N' && lineCur[5] == 's' && drmtl_wavefront_is_whitespace(lineCur[6]))     // Specular exponent map
                            {
                                lineCur += 7;
                                drmtl_wavefront_parse_map(lineCur, lineEnd, pWavefront->specularExponentMap, DRMTL_MAX_INPUT_PATH);
                            }
                            else if (lineCur[4] == 'd' && drmtl_wavefront_is_whitespace(lineCur[5]))                          // Opacity/Alpha map
                            {
                                lineCur += 6;
                                drmtl_wavefront_parse_map(lineCur, lineEnd, pWavefront->alphaMap, DRMTL_MAX_INPUT_PATH);
                            }
                        }
                    }
                }
            }


            // Move to the end of the line.
            pWavefront->pDataCur = lineEnd;

            // Move to the start of the next line. If this fails it probably means we've reached the end of the data so we just break from the loop
            if (!drmtl_wavefront_seek_to_next_line(pWavefront))
            {
                break;
            }
        }


        return 1;
    }

    return 0;
}

bool drmtl_wavefront_compile(drmtl_material* pMaterial, drmtl_wavefront* pWavefront, const char* texcoordInputName)
{
    assert(pMaterial  != NULL);
    assert(pWavefront != NULL);

    unsigned int texCoordID;    // Private input for texture coordinates.
    unsigned int diffuseID;
    unsigned int specularID;
    unsigned int specularExponentID;
    unsigned int alphaID;
    unsigned int diffuseMapID = (unsigned int)-1;
    unsigned int specularMapID = (unsigned int)-1;
    unsigned int specularExponentMapID = (unsigned int)-1;
    unsigned int alphaMapID = (unsigned int)-1;
    unsigned int diffuseResultID = (unsigned int)-1;
    unsigned int specularResultID = (unsigned int)-1;
    unsigned int specularExponentResultID = (unsigned int)-1;
    unsigned int alphaResultID = (unsigned int)-1;


    // Identifiers.
    drmtl_appendidentifier(pMaterial, drmtl_identifier_float2(texcoordInputName), &texCoordID);
    drmtl_appendidentifier(pMaterial, drmtl_identifier_float4("DiffuseColor"), &diffuseID);
    drmtl_appendidentifier(pMaterial, drmtl_identifier_float3("SpecularColor"), &specularID);
    drmtl_appendidentifier(pMaterial, drmtl_identifier_float("SpecularExponent"), &specularExponentID);
    drmtl_appendidentifier(pMaterial, drmtl_identifier_float("Alpha"), &alphaID);

    if (pWavefront->diffuseMap[0] != '\0') {
        drmtl_appendidentifier(pMaterial, drmtl_identifier_tex2d("DiffuseMap"), &diffuseMapID);
        drmtl_appendidentifier(pMaterial, drmtl_identifier_float4("DiffuseResult"), &diffuseResultID);
    }
    if (pWavefront->specularMap[0] != '\0') {
        drmtl_appendidentifier(pMaterial, drmtl_identifier_tex2d("SpecularMap"), &specularMapID);
        drmtl_appendidentifier(pMaterial, drmtl_identifier_float4("SpecularResult"), &specularResultID);
    }
    if (pWavefront->specularExponentMap[0] != '\0') {
        drmtl_appendidentifier(pMaterial, drmtl_identifier_tex2d("SpecularExponentMap"), &specularExponentMapID);
        drmtl_appendidentifier(pMaterial, drmtl_identifier_float4("SpecularExponentResult"), &specularExponentResultID);
    }
    if (pWavefront->alphaMap[0] != '\0') {
        drmtl_appendidentifier(pMaterial, drmtl_identifier_tex2d("AlphaMap"), &alphaMapID);
        drmtl_appendidentifier(pMaterial, drmtl_identifier_float4("AlphaResult"), &alphaResultID);
    }


    // Inputs.
    drmtl_appendprivateinput(pMaterial, drmtl_input_float2(texCoordID, 0, 0));
    drmtl_appendpublicinput(pMaterial, drmtl_input_float4(diffuseID, pWavefront->diffuse[0], pWavefront->diffuse[1], pWavefront->diffuse[2], 1.0f));
    drmtl_appendpublicinput(pMaterial, drmtl_input_float3(specularID, pWavefront->specular[0], pWavefront->specular[1], pWavefront->specular[2]));
    drmtl_appendpublicinput(pMaterial, drmtl_input_float(specularExponentID, pWavefront->specularExponent));
    drmtl_appendpublicinput(pMaterial, drmtl_input_float(alphaID, pWavefront->alpha));

    if (pWavefront->diffuseMap[0] != '\0') {
        drmtl_appendpublicinput(pMaterial, drmtl_input_tex(diffuseMapID, pWavefront->diffuseMap));
    }
    if (pWavefront->specularMap[0] != '\0') {
        drmtl_appendpublicinput(pMaterial, drmtl_input_tex(specularMapID, pWavefront->specularMap));
    }
    if (pWavefront->specularExponentMap[0] != '\0') {
        drmtl_appendpublicinput(pMaterial, drmtl_input_tex(specularExponentMapID, pWavefront->specularExponentMap));
    }
    if (pWavefront->alphaMap[0] != '\0') {
        drmtl_appendpublicinput(pMaterial, drmtl_input_tex(alphaMapID, pWavefront->alphaMap));
    }


    // Channels.
    drmtl_appendchannel(pMaterial, drmtl_channel_float4("DiffuseChannel"));
    if (pWavefront->diffuseMap[0] != '\0') {
        drmtl_appendinstruction(pMaterial, drmtl_var(diffuseResultID));
        drmtl_appendinstruction(pMaterial, drmtl_tex2(diffuseResultID, diffuseMapID, texCoordID));
        drmtl_appendinstruction(pMaterial, drmtl_mulf4_v3c1(diffuseResultID, diffuseID, 1.0f));
        drmtl_appendinstruction(pMaterial, drmtl_retf4(diffuseResultID));
    } else {
        drmtl_appendinstruction(pMaterial, drmtl_retf4(diffuseID));
    }

    drmtl_appendchannel(pMaterial, drmtl_channel_float3("SpecularChannel"));
    if (pWavefront->specularMap[0] != '\0') {
        drmtl_appendinstruction(pMaterial, drmtl_var(specularResultID));
        drmtl_appendinstruction(pMaterial, drmtl_tex2(specularResultID, specularMapID, texCoordID));
        drmtl_appendinstruction(pMaterial, drmtl_mulf4_v3c1(specularResultID, specularID, 1.0f));
        drmtl_appendinstruction(pMaterial, drmtl_retf3(specularResultID));
    } else {
        drmtl_appendinstruction(pMaterial, drmtl_retf3(specularID));
    }

    drmtl_appendchannel(pMaterial, drmtl_channel_float("SpecularExponentChannel"));
    if (pWavefront->specularExponentMap[0] != '\0') {
        drmtl_appendinstruction(pMaterial, drmtl_var(specularExponentResultID));
        drmtl_appendinstruction(pMaterial, drmtl_tex2(specularResultID, specularMapID, texCoordID));
        drmtl_appendinstruction(pMaterial, drmtl_mulf4_v1c3(specularResultID, specularID, 1.0f, 1.0f, 1.0f));
        drmtl_appendinstruction(pMaterial, drmtl_retf1(specularResultID));
    } else {
        drmtl_appendinstruction(pMaterial, drmtl_retf1(specularExponentID));
    }

    drmtl_appendchannel(pMaterial, drmtl_channel_float("AlphaChannel"));
    if (pWavefront->alphaMap[0] != '\0') {
        drmtl_appendinstruction(pMaterial, drmtl_var(alphaResultID));
        drmtl_appendinstruction(pMaterial, drmtl_tex2(alphaResultID, alphaMapID, texCoordID));
        drmtl_appendinstruction(pMaterial, drmtl_mulf4_v1c3(alphaResultID, alphaID, 1.0f, 1.0f, 1.0f));
        drmtl_appendinstruction(pMaterial, drmtl_retf1(alphaResultID));
    } else {
        drmtl_appendinstruction(pMaterial, drmtl_retf1(alphaID));
    }



    // Properties.
    if (pWavefront->alphaMap[0] != '\0' || pWavefront->alpha < 1)
    {
        drmtl_appendproperty(pMaterial, drmtl_property_bool("IsTransparent", 1));
    }

    return 1;
}


bool drmtl_compile_wavefront_mtl(drmtl_material* pMaterial, const char* mtlData, size_t mtlDataSizeInBytes, const char* texcoordInputName)
{
    if (pMaterial != NULL && mtlData != NULL && mtlDataSizeInBytes > 0)
    {
        if (drmtl_init(pMaterial))
        {
            drmtl_wavefront wavefront;
            wavefront.pData                  = mtlData;
            wavefront.dataSizeInBytes        = mtlDataSizeInBytes;
            wavefront.pDataCur               = wavefront.pData;
            wavefront.pDataEnd               = wavefront.pData + wavefront.dataSizeInBytes;
            wavefront.diffuse[0]             = 1; wavefront.diffuse[1] = 1;  wavefront.diffuse[2] = 1;
            wavefront.diffuseMap[0]          = '\0';
            wavefront.specular[0]            = 1; wavefront.specular[1] = 1; wavefront.specular[2] = 1;
            wavefront.specularMap[0]         = '\0';
            wavefront.specularExponent       = 10;
            wavefront.specularExponentMap[0] = '\0';
            wavefront.alpha                  = 1;
            wavefront.alphaMap[0]            = '\0';

            if (drmtl_wavefront_parse(&wavefront))
            {
                if (drmtl_wavefront_compile(pMaterial, &wavefront, texcoordInputName))
                {
                    return 1;
                }
                else
                {
                    // Failed to compile.
                    drmtl_uninit(pMaterial);
                }
            }
            else
            {
                // Failed to parse the file.
                drmtl_uninit(pMaterial);
            }
        }
    }

    return 0;
}
#endif




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Code Generators
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef DRMTL_NO_GLSL_CODEGEN
#include <stdio.h>

#if defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-enum"
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #pragma GCC diagnostic ignored "-Wused-but-marked-unused"   // This ie emitted for snprintf() for some reason. Need to investigate...
#endif

typedef struct
{
    /// The output buffer. Can be null, in which case nothing is written.
    char* pBufferOut;

    /// The size of the output buffer.
    unsigned int bufferOutSizeInBytes;

    /// The current length of the string copied to the output buffer.
    unsigned int runningLength;


    /// A pointer to the material that is being used as the source of the code generation.
    drmtl_material* pMaterial;

    /// A pointer to the buffer containing the material's identifiers.
    drmtl_identifier* pIdentifiers;

    /// The number of identifiers.
    unsigned int identifierCount;


    /// The current indentation level, in spaces.
    unsigned int indentationLevel;

} drmtl_codegen_glsl;

bool drmtl_codegen_glsl_write(drmtl_codegen_glsl* pCodegen, const char* src)
{
    assert(pCodegen != NULL);
    assert(src      != NULL);

    if (pCodegen->pBufferOut != NULL)
    {
        unsigned int dstSizeInBytes = (pCodegen->bufferOutSizeInBytes - pCodegen->runningLength);
        while (dstSizeInBytes > 0 && src[0] != '\0')
        {
            pCodegen->pBufferOut[pCodegen->runningLength + 0] = src[0];

            pCodegen->runningLength += 1;
            src += 1;
            dstSizeInBytes -= 1;
        }

        if (dstSizeInBytes > 0)
        {
            // There's enough room for the null terminator which means there was enough room in the buffer. All good.
            pCodegen->pBufferOut[pCodegen->runningLength] = '\0';
            return 1;
        }
        else
        {
            // There's not enough room for the null terminator which means there was NOT enough room in the buffer. Error.
            return 0;
        }
    }
    else
    {
        // We're just measuring.
        pCodegen->runningLength += (unsigned int)strlen(src);
        return 1;
    }
}

bool drmtl_codegen_glsl_write_float(drmtl_codegen_glsl* pCodegen, float src)
{
    assert(pCodegen != NULL);

    char str[32];
    if (snprintf(str, 32, "%f", src) > 0)
    {
        return drmtl_codegen_glsl_write(pCodegen, str);
    }
    else
    {
        return 0;
    }
}

bool drmtl_codegen_glsl_write_int(drmtl_codegen_glsl* pCodegen, int src)
{
    assert(pCodegen != NULL);

    char str[32];
    if (snprintf(str, 32, "%d", src) > 0)
    {
        return drmtl_codegen_glsl_write(pCodegen, str);
    }
    else
    {
        return 0;
    }
}

bool drmtl_codegen_glsl_write_indentation(drmtl_codegen_glsl* pCodegen)
{
    assert(pCodegen != NULL);

    for (unsigned int i = 0; i < pCodegen->indentationLevel; ++i)
    {
        drmtl_codegen_glsl_write(pCodegen, " ");
    }

    return 1;
}

bool drmtl_codegen_glsl_write_type(drmtl_codegen_glsl* pCodegen, drmtl_type type)
{
    assert(pCodegen != NULL);

    switch (type)
    {
    case drmtl_type_float:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "float"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_float2:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "vec2"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_float3:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "vec3"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_float4:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "vec4"))
            {
                return 0;
            }

            break;
        }

    case drmtl_type_int:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "int"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_int2:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "ivec2"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_int3:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "ivec3"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_int4:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "ivec4"))
            {
                return 0;
            }

            break;
        }

    case drmtl_type_tex1d:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "sampler1D"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_tex2d:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "sampler2D"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_tex3d:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "sampler3D"))
            {
                return 0;
            }

            break;
        }
    case drmtl_type_texcube:
        {
            if (!drmtl_codegen_glsl_write(pCodegen, "samplerCube"))
            {
                return 0;
            }

            break;
        }

    default:
        {
            // Unsupported return type.
            return 0;
        }
    }

    return 1;
}

bool drmtl_codegen_glsl_write_instruction_input_scalar(drmtl_codegen_glsl* pCodegen, unsigned char descriptor, drmtl_instruction_input* pInput)
{
    assert(pCodegen != NULL);
    assert(pInput   != NULL);

    if (descriptor == DRMTL_INPUT_DESC_CONSTF)
    {
        // It's a constant float.
        return drmtl_codegen_glsl_write_float(pCodegen, pInput->valuef);
    }
    else if (descriptor == DRMTL_INPUT_DESC_CONSTI)
    {
        // It's a constant int.
        return drmtl_codegen_glsl_write_int(pCodegen, pInput->valuei);
    }
    else
    {
        // It's a variable.
        if (pInput->id < pCodegen->identifierCount)
        {
            drmtl_identifier* pIdentifier = pCodegen->pIdentifiers + pInput->id;
            assert(pIdentifier != NULL);

            if (pIdentifier->type == drmtl_type_float)
            {
                // The input variable is a float, so we don't want to use any selectors.
                return drmtl_codegen_glsl_write(pCodegen, pIdentifier->name);
            }
            else
            {
                if (drmtl_codegen_glsl_write(pCodegen, pIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, "."))
                {
                    switch (descriptor)
                    {
                    case 0: return drmtl_codegen_glsl_write(pCodegen, "x");
                    case 1: return drmtl_codegen_glsl_write(pCodegen, "y");
                    case 2: return drmtl_codegen_glsl_write(pCodegen, "z");
                    case 3: return drmtl_codegen_glsl_write(pCodegen, "w");
                    default: return 0;
                    }
                }
            }
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_input_initializer(drmtl_codegen_glsl* pCodegen, drmtl_type type, drmtl_instruction_input_descriptor inputDesc, drmtl_instruction_input* pInputs)
{
    assert(pCodegen != NULL);
    assert(pInputs  != NULL);

    switch (type)
    {
    case drmtl_type_float:
        {
            return drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0);
        }

    case drmtl_type_float2:
        {
            if (drmtl_codegen_glsl_write(pCodegen, "vec2("))
            {
                if (drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.y, pInputs + 1))
                {
                    return drmtl_codegen_glsl_write(pCodegen, ")");
                }
            }

            break;
        }

    case drmtl_type_float3:
        {
            if (drmtl_codegen_glsl_write(pCodegen, "vec3("))
            {
                if (drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.y, pInputs + 1) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.z, pInputs + 2))
                {
                    return drmtl_codegen_glsl_write(pCodegen, ")");
                }
            }

            break;
        }

    case drmtl_type_float4:
        {
            if (drmtl_codegen_glsl_write(pCodegen, "vec4("))
            {
                if (drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.y, pInputs + 1) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.z, pInputs + 2) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.w, pInputs + 3))
                {
                    return drmtl_codegen_glsl_write(pCodegen, ")");
                }
            }

            break;
        }


    case drmtl_type_int:
        {
            return drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0);
        }

    case drmtl_type_int2:
        {
            if (drmtl_codegen_glsl_write(pCodegen, "ivec2("))
            {
                if (drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.y, pInputs + 1))
                {
                    return drmtl_codegen_glsl_write(pCodegen, ")");
                }
            }

            break;
        }

    case drmtl_type_int3:
        {
            if (drmtl_codegen_glsl_write(pCodegen, "ivec3("))
            {
                if (drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.y, pInputs + 1) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.z, pInputs + 2))
                {
                    return drmtl_codegen_glsl_write(pCodegen, ")");
                }
            }

            break;
        }

    case drmtl_type_int4:
        {
            if (drmtl_codegen_glsl_write(pCodegen, "ivec4("))
            {
                if (drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.x, pInputs + 0) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.y, pInputs + 1) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.z, pInputs + 2) && drmtl_codegen_glsl_write(pCodegen, ", ") &&
                    drmtl_codegen_glsl_write_instruction_input_scalar(pCodegen, inputDesc.w, pInputs + 3))
                {
                    return drmtl_codegen_glsl_write(pCodegen, ")");
                }
            }

            break;
        }


    default:
        {
            // Unsupported return type.
            return 0;
        }
    }

    return 0;
}


bool drmtl_codegen_glsl_write_instruction_mov(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->mov.output < pCodegen->identifierCount)
    {
        drmtl_identifier* pOutputIdentifier = pCodegen->pIdentifiers + pInstruction->mov.output;
        assert(pOutputIdentifier != NULL);

        if (drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, " = "))
        {
            drmtl_type type;
            switch (pInstruction->opcode)
            {
            case drmtl_opcode_movf1: type = drmtl_type_float;  break;
            case drmtl_opcode_movf2: type = drmtl_type_float2; break;
            case drmtl_opcode_movf3: type = drmtl_type_float3; break;
            case drmtl_opcode_movf4: type = drmtl_type_float4; break;
            case drmtl_opcode_movi1: type = drmtl_type_int;    break;
            case drmtl_opcode_movi2: type = drmtl_type_int2;   break;
            case drmtl_opcode_movi3: type = drmtl_type_int3;   break;
            case drmtl_opcode_movi4: type = drmtl_type_int4;   break;
            default: return 0;
            }

            return drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->mov.inputDesc, &pInstruction->mov.inputX) && drmtl_codegen_glsl_write(pCodegen, ";\n");
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_add(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->add.output < pCodegen->identifierCount)
    {
        drmtl_identifier* pOutputIdentifier = pCodegen->pIdentifiers + pInstruction->add.output;
        assert(pOutputIdentifier != NULL);

        if (drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, " += "))
        {
            drmtl_type type;
            switch (pInstruction->opcode)
            {
            case drmtl_opcode_addf1: type = drmtl_type_float;  break;
            case drmtl_opcode_addf2: type = drmtl_type_float2; break;
            case drmtl_opcode_addf3: type = drmtl_type_float3; break;
            case drmtl_opcode_addf4: type = drmtl_type_float4; break;
            case drmtl_opcode_addi1: type = drmtl_type_int;    break;
            case drmtl_opcode_addi2: type = drmtl_type_int2;   break;
            case drmtl_opcode_addi3: type = drmtl_type_int3;   break;
            case drmtl_opcode_addi4: type = drmtl_type_int4;   break;
            default: return 0;
            }

            return drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->add.inputDesc, &pInstruction->add.inputX) && drmtl_codegen_glsl_write(pCodegen, ";\n");
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_sub(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->add.output < pCodegen->identifierCount)
    {
        drmtl_identifier* pOutputIdentifier = pCodegen->pIdentifiers + pInstruction->sub.output;
        assert(pOutputIdentifier != NULL);

        if (drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, " -= "))
        {
            drmtl_type type;
            switch (pInstruction->opcode)
            {
            case drmtl_opcode_subf1: type = drmtl_type_float;  break;
            case drmtl_opcode_subf2: type = drmtl_type_float2; break;
            case drmtl_opcode_subf3: type = drmtl_type_float3; break;
            case drmtl_opcode_subf4: type = drmtl_type_float4; break;
            case drmtl_opcode_subi1: type = drmtl_type_int;    break;
            case drmtl_opcode_subi2: type = drmtl_type_int2;   break;
            case drmtl_opcode_subi3: type = drmtl_type_int3;   break;
            case drmtl_opcode_subi4: type = drmtl_type_int4;   break;
            default: return 0;
            }

            return drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->sub.inputDesc, &pInstruction->sub.inputX) && drmtl_codegen_glsl_write(pCodegen, ";\n");
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_mul(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->mul.output < pCodegen->identifierCount)
    {
        drmtl_identifier* pOutputIdentifier = pCodegen->pIdentifiers + pInstruction->mul.output;
        assert(pOutputIdentifier != NULL);

        if (drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, " *= "))
        {
            drmtl_type type;
            switch (pInstruction->opcode)
            {
            case drmtl_opcode_mulf1: type = drmtl_type_float;  break;
            case drmtl_opcode_mulf2: type = drmtl_type_float2; break;
            case drmtl_opcode_mulf3: type = drmtl_type_float3; break;
            case drmtl_opcode_mulf4: type = drmtl_type_float4; break;
            case drmtl_opcode_muli1: type = drmtl_type_int;    break;
            case drmtl_opcode_muli2: type = drmtl_type_int2;   break;
            case drmtl_opcode_muli3: type = drmtl_type_int3;   break;
            case drmtl_opcode_muli4: type = drmtl_type_int4;   break;
            default: return 0;
            }

            return drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->mul.inputDesc, &pInstruction->mul.inputX) && drmtl_codegen_glsl_write(pCodegen, ";\n");
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_div(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->div.output < pCodegen->identifierCount)
    {
        drmtl_identifier* pOutputIdentifier = pCodegen->pIdentifiers + pInstruction->div.output;
        assert(pOutputIdentifier != NULL);

        if (drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, " = "))
        {
            drmtl_type type;
            switch (pInstruction->opcode)
            {
            case drmtl_opcode_divf1: type = drmtl_type_float;  break;
            case drmtl_opcode_divf2: type = drmtl_type_float2; break;
            case drmtl_opcode_divf3: type = drmtl_type_float3; break;
            case drmtl_opcode_divf4: type = drmtl_type_float4; break;
            case drmtl_opcode_divi1: type = drmtl_type_int;    break;
            case drmtl_opcode_divi2: type = drmtl_type_int2;   break;
            case drmtl_opcode_divi3: type = drmtl_type_int3;   break;
            case drmtl_opcode_divi4: type = drmtl_type_int4;   break;
            default: return 0;
            }

            return drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->div.inputDesc, &pInstruction->div.inputX) && drmtl_codegen_glsl_write(pCodegen, ";\n");
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_pow(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->pow.output < pCodegen->identifierCount)
    {
        drmtl_identifier* pOutputIdentifier = pCodegen->pIdentifiers + pInstruction->pow.output;
        assert(pOutputIdentifier != NULL);

        if (drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, " = pow(") && drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, ", "))
        {
            drmtl_type type;
            switch (pInstruction->opcode)
            {
            case drmtl_opcode_powf1: type = drmtl_type_float;  break;
            case drmtl_opcode_powf2: type = drmtl_type_float2; break;
            case drmtl_opcode_powf3: type = drmtl_type_float3; break;
            case drmtl_opcode_powf4: type = drmtl_type_float4; break;
            case drmtl_opcode_powi1: type = drmtl_type_int;    break;
            case drmtl_opcode_powi2: type = drmtl_type_int2;   break;
            case drmtl_opcode_powi3: type = drmtl_type_int3;   break;
            case drmtl_opcode_powi4: type = drmtl_type_int4;   break;
            default: return 0;
            }

            return drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->pow.inputDesc, &pInstruction->pow.inputX) && drmtl_codegen_glsl_write(pCodegen, ");\n");
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_tex(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->tex.output < pCodegen->identifierCount && pInstruction->tex.texture < pCodegen->identifierCount)
    {
        drmtl_identifier* pOutputIdentifier = pCodegen->pIdentifiers + pInstruction->tex.output;
        assert(pOutputIdentifier != NULL);

        drmtl_identifier* pTextureIdentifier = pCodegen->pIdentifiers + pInstruction->tex.texture;
        assert(pTextureIdentifier != NULL);

        if (drmtl_codegen_glsl_write(pCodegen, pOutputIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, " = "))
        {
            drmtl_type type;
            switch (pInstruction->opcode)
            {
            case drmtl_opcode_tex1:
            {
                type = drmtl_type_float;
                if (!drmtl_codegen_glsl_write(pCodegen, "texture1D("))
                {
                    return 0;
                }

                break;
            }

            case drmtl_opcode_tex2:
            {
                type = drmtl_type_float2;
                if (!drmtl_codegen_glsl_write(pCodegen, "texture2D("))
                {
                    return 0;
                }

                break;
            }

            case drmtl_opcode_tex3:
            {
                type = drmtl_type_float3;
                if (!drmtl_codegen_glsl_write(pCodegen, "texture3D("))
                {
                    return 0;
                }

                break;
            }

            case drmtl_opcode_texcube:
            {
                type = drmtl_type_float3;
                if (!drmtl_codegen_glsl_write(pCodegen, "textureCube("))
                {
                    return 0;
                }

                break;
            }

            default: return 0;
            }

            return
                drmtl_codegen_glsl_write(pCodegen, pTextureIdentifier->name) &&
                drmtl_codegen_glsl_write(pCodegen, ", ") &&
                drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->tex.inputDesc, &pInstruction->tex.inputX) &&
                drmtl_codegen_glsl_write(pCodegen, ");\n");
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_var(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (pInstruction->var.identifierIndex < pCodegen->identifierCount)
    {
        drmtl_identifier* pIdentifier = pCodegen->pIdentifiers + pInstruction->var.identifierIndex;
        assert(pIdentifier != NULL);

        return drmtl_codegen_glsl_write_type(pCodegen, pIdentifier->type) && drmtl_codegen_glsl_write(pCodegen, " ") && drmtl_codegen_glsl_write(pCodegen, pIdentifier->name) && drmtl_codegen_glsl_write(pCodegen, ";\n");
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction_ret(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (drmtl_codegen_glsl_write(pCodegen, "return "))
    {
        drmtl_type type;
        switch (pInstruction->opcode)
        {
        case drmtl_opcode_retf1: type = drmtl_type_float;  break;
        case drmtl_opcode_retf2: type = drmtl_type_float2; break;
        case drmtl_opcode_retf3: type = drmtl_type_float3; break;
        case drmtl_opcode_retf4: type = drmtl_type_float4; break;
        case drmtl_opcode_reti1: type = drmtl_type_int;    break;
        case drmtl_opcode_reti2: type = drmtl_type_int2;   break;
        case drmtl_opcode_reti3: type = drmtl_type_int3;   break;
        case drmtl_opcode_reti4: type = drmtl_type_int4;   break;
        default: return 0;
        }

        return drmtl_codegen_glsl_write_instruction_input_initializer(pCodegen, type, pInstruction->ret.inputDesc, &pInstruction->ret.inputX) && drmtl_codegen_glsl_write(pCodegen, ";\n");
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instruction(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstruction)
{
    assert(pCodegen     != NULL);
    assert(pInstruction != NULL);

    if (drmtl_codegen_glsl_write_indentation(pCodegen))
    {
        switch (pInstruction->opcode)
        {
        case drmtl_opcode_movf1:
        case drmtl_opcode_movf2:
        case drmtl_opcode_movf3:
        case drmtl_opcode_movf4:
        case drmtl_opcode_movi1:
        case drmtl_opcode_movi2:
        case drmtl_opcode_movi3:
        case drmtl_opcode_movi4:
            {
                return drmtl_codegen_glsl_write_instruction_mov(pCodegen, pInstruction);
            }


        case drmtl_opcode_addf1:
        case drmtl_opcode_addf2:
        case drmtl_opcode_addf3:
        case drmtl_opcode_addf4:
        case drmtl_opcode_addi1:
        case drmtl_opcode_addi2:
        case drmtl_opcode_addi3:
        case drmtl_opcode_addi4:
            {
                return drmtl_codegen_glsl_write_instruction_add(pCodegen, pInstruction);
            }

        case drmtl_opcode_subf1:
        case drmtl_opcode_subf2:
        case drmtl_opcode_subf3:
        case drmtl_opcode_subf4:
        case drmtl_opcode_subi1:
        case drmtl_opcode_subi2:
        case drmtl_opcode_subi3:
        case drmtl_opcode_subi4:
            {
                return drmtl_codegen_glsl_write_instruction_sub(pCodegen, pInstruction);
            }

        case drmtl_opcode_mulf1:
        case drmtl_opcode_mulf2:
        case drmtl_opcode_mulf3:
        case drmtl_opcode_mulf4:
        case drmtl_opcode_muli1:
        case drmtl_opcode_muli2:
        case drmtl_opcode_muli3:
        case drmtl_opcode_muli4:
            {
                return drmtl_codegen_glsl_write_instruction_mul(pCodegen, pInstruction);
            }

        case drmtl_opcode_divf1:
        case drmtl_opcode_divf2:
        case drmtl_opcode_divf3:
        case drmtl_opcode_divf4:
        case drmtl_opcode_divi1:
        case drmtl_opcode_divi2:
        case drmtl_opcode_divi3:
        case drmtl_opcode_divi4:
            {
                return drmtl_codegen_glsl_write_instruction_div(pCodegen, pInstruction);
            }

        case drmtl_opcode_powf1:
        case drmtl_opcode_powf2:
        case drmtl_opcode_powf3:
        case drmtl_opcode_powf4:
        case drmtl_opcode_powi1:
        case drmtl_opcode_powi2:
        case drmtl_opcode_powi3:
        case drmtl_opcode_powi4:
            {
                return drmtl_codegen_glsl_write_instruction_pow(pCodegen, pInstruction);
            }

        case drmtl_opcode_tex1:
        case drmtl_opcode_tex2:
        case drmtl_opcode_tex3:
        case drmtl_opcode_texcube:
            {
                return drmtl_codegen_glsl_write_instruction_tex(pCodegen, pInstruction);
            }


        case drmtl_opcode_var:
            {
                return drmtl_codegen_glsl_write_instruction_var(pCodegen, pInstruction);
            }

        case drmtl_opcode_retf1:
        case drmtl_opcode_retf2:
        case drmtl_opcode_retf3:
        case drmtl_opcode_retf4:
        case drmtl_opcode_reti1:
        case drmtl_opcode_reti2:
        case drmtl_opcode_reti3:
        case drmtl_opcode_reti4:
            {
                return drmtl_codegen_glsl_write_instruction_ret(pCodegen, pInstruction);
            }


        default:
            {
                // Unknown or unsupported opcode.
                break;
            }
        }
    }

    return 0;
}

bool drmtl_codegen_glsl_write_instructions(drmtl_codegen_glsl* pCodegen, drmtl_instruction* pInstructions, unsigned int instructionCount)
{
    assert(pCodegen      != NULL);
    assert(pInstructions != NULL);

    for (unsigned int iInstruction = 0; iInstruction < instructionCount; ++iInstruction)
    {
        drmtl_instruction* pInstruction = pInstructions + iInstruction;
        assert(pInstruction != NULL);

        if (!drmtl_codegen_glsl_write_instruction(pCodegen, pInstruction))
        {
            return 0;
        }
    }

    return 1;
}

bool drmtl_codegen_glsl_channel_function_begin(drmtl_codegen_glsl* pCodegen, drmtl_channel_header* pChannelHeader)
{
    assert(pCodegen       != NULL);
    assert(pChannelHeader != NULL);

    // <type> <name> {\n
    bool result =
        drmtl_codegen_glsl_write_type(pCodegen, pChannelHeader->channel.type) &&
        drmtl_codegen_glsl_write(pCodegen, " ") &&
        drmtl_codegen_glsl_write(pCodegen, pChannelHeader->channel.name) &&
        drmtl_codegen_glsl_write(pCodegen, "() {\n");
    if (result)
    {
        pCodegen->indentationLevel += 4;
    }

    return result;
}

bool drmtl_codegen_glsl_channel_function_close(drmtl_codegen_glsl* pCodegen)
{
    assert(pCodegen != NULL);

    if (pCodegen->indentationLevel > 4) {
        pCodegen->indentationLevel -= 4;
    } else {
        pCodegen->indentationLevel = 0;
    }

    return drmtl_codegen_glsl_write(pCodegen, "}\n");
}

bool drmtl_codegen_glsl_channel(drmtl_material* pMaterial, const char* channelName, char* codeOut, size_t codeOutSizeInBytes, size_t* pBytesWrittenOut)
{
    if (pMaterial != NULL)
    {
        drmtl_header* pHeader = drmtl_getheader(pMaterial);
        if (pHeader != NULL)
        {
            drmtl_channel_header* pChannelHeader = drmtl_getchannelheaderbyname(pMaterial, channelName);
            if (pChannelHeader != NULL)
            {
                drmtl_codegen_glsl codegen;
                codegen.pBufferOut           = codeOut;
                codegen.bufferOutSizeInBytes = codeOutSizeInBytes;
                codegen.runningLength        = 0;
                codegen.pMaterial            = pMaterial;
                codegen.pIdentifiers         = drmtl_getidentifiers(pMaterial);
                codegen.identifierCount      = drmtl_getidentifiercount(pMaterial);
                codegen.indentationLevel     = 0;

                if (drmtl_codegen_glsl_channel_function_begin(&codegen, pChannelHeader))
                {
                    drmtl_instruction* pInstructions = (drmtl_instruction*)(pChannelHeader + 1);
                    assert(pInstructions != NULL);

                    if (drmtl_codegen_glsl_write_instructions(&codegen, pInstructions, pChannelHeader->instructionCount))
                    {
                        bool result = drmtl_codegen_glsl_channel_function_close(&codegen);
                        if (result)
                        {
                            if (pBytesWrittenOut != NULL)
                            {
                                *pBytesWrittenOut = codegen.runningLength + 1;
                            }
                        }

                        return result;
                    }
                }
            }
        }
    }

    return 0;
}



bool drmtl_codegen_glsl_uniform(drmtl_codegen_glsl* pCodegen, drmtl_input* pInput)
{
    assert(pCodegen != NULL);
    assert(pInput   != NULL);

    if (pInput->identifierIndex < pCodegen->identifierCount)
    {
        drmtl_identifier* pIdentifier = pCodegen->pIdentifiers + pInput->identifierIndex;
        assert(pIdentifier != NULL);

        // uniform <type> <name>;
        return
            drmtl_codegen_glsl_write(pCodegen, "uniform ") &&
            drmtl_codegen_glsl_write_type(pCodegen, pIdentifier->type) &&
            drmtl_codegen_glsl_write(pCodegen, " ") &&
            drmtl_codegen_glsl_write(pCodegen, pIdentifier->name) &&
            drmtl_codegen_glsl_write(pCodegen, ";\n");
    }

    return 0;
}

bool drmtl_codegen_glsl_uniforms(drmtl_material* pMaterial, char* codeOut, size_t codeOutSizeInBytes, size_t* pBytesWritteOut)
{
    if (pMaterial != NULL)
    {
        drmtl_codegen_glsl codegen;
        codegen.pBufferOut           = codeOut;
        codegen.bufferOutSizeInBytes = codeOutSizeInBytes;
        codegen.runningLength        = 0;
        codegen.pMaterial            = pMaterial;
        codegen.pIdentifiers         = drmtl_getidentifiers(pMaterial);
        codegen.identifierCount      = drmtl_getidentifiercount(pMaterial);
        codegen.indentationLevel     = 0;


        unsigned int inputCount = drmtl_getpublicinputcount(pMaterial);
        if (inputCount > 0)
        {
            for (unsigned int iInput = 0; iInput < inputCount; ++iInput)
            {
                drmtl_input* pInput = drmtl_getpublicinputbyindex(pMaterial, iInput);
                assert(pInput != NULL);

                if (!drmtl_codegen_glsl_uniform(&codegen, pInput))
                {
                    // There was an error writing one of the uniforms. Return false.
                    return 0;
                }
            }
        }
        else
        {
            // No inputs. Just write an empty string.
            drmtl_codegen_glsl_write(&codegen, "");
        }

        if (pBytesWritteOut != NULL)
        {
            *pBytesWritteOut = codegen.runningLength + 1;
        }

        return 1;
    }

    return 0;
}

#if defined(__clang__)
    #pragma GCC diagnostic pop
#endif
#endif

#if defined(__clang__)
    #pragma GCC diagnostic pop
#endif
#endif

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
