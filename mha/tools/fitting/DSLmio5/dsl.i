%module dsl
%{
#include <dsl/channel_array.h>
#include <dsl/container.h>
#include <dsl/constants.h>
#include <dsl/dslmio.h>
#include <dsl/input_signal.h>
#include <stdio.h>
dsl_datacontainer_t* dsl_datacontainer_null() {return NULL;}
dsl_container_t* dsl_container_null() {return NULL;}
int is_datacontainer_null(const dsl_datacontainer_t* p) {return p==NULL;}
int is_container_null(const dsl_container_t* p) {return p==NULL;}
int dsl_number_of_all_valid_frequencies() {return NUMBER_OF_ALL_VALID_FREQUENCIES;}
dsl_frequency_t dsl_get_valid_frequency(int i) {return ALL_VALID_FREQUENCIES[i];}
dsl_venting_correction_t * dsl_venting_correction_null() {return NULL;}
%}


// Definitions for dsl_channel_array functions

typedef enum {
    DSL_OK = 0x0,               ///< No error everything is OK
    DSL_FAILED,                 ///< Something has failed function did not execute
    DSL_ALLOCATION_ERROR,       ///< Allocation error (can't allocate memory)
    DSL_LEVEL_NOT_FOUND,        ///< Level does not exist for this frequency
    DSL_TABLE_OUT_OF_MEMORY,    ///< Can't load Table of values
    DSL_TABLE_MISSING_RECORDS,  ///< Can't load Record for Table
    DSL_CHANNEL_LIMIT_EXCEEDED, ///< Frequency not in a channel
    DSL_MISSING_PARAMETER       ///< Missing parameter or structure for the function called
} dsl_error_t;

typedef struct _dsl_channel_array_t dsl_channel_array_t;
typedef unsigned short  dsl_frequency_t;

dsl_channel_array_t* dsl_channel_array_new();
dsl_channel_array_t* dsl_channel_array_copy (const dsl_channel_array_t* channels);
void                 dsl_channel_array_delete (dsl_channel_array_t* channel_array);
dsl_error_t          dsl_channel_array_add_crossover_frequency (dsl_channel_array_t* channel_array, dsl_frequency_t frequency);


// Definitions for dsl_container functions

typedef struct _dsl_container_t dsl_container_t;
typedef double dsl_level_t;

dsl_container_t* dsl_container_new             ();
dsl_container_t* dsl_container_copy            (const dsl_container_t* container);
void             dsl_container_delete          (dsl_container_t* container);
dsl_error_t      dsl_container_get_level_at_frequency    (const dsl_container_t* container, dsl_frequency_t frequency, dsl_level_t* OUTPUT);
dsl_error_t      dsl_container_interpolate                         (dsl_container_t* container);
size_t           dsl_container_size            (const dsl_container_t* container);
dsl_error_t      dsl_container_insert_level              (dsl_container_t* container, dsl_frequency_t frequency, dsl_level_t dB);
dsl_container_t* dsl_container_null();
int is_container_null(const dsl_container_t* p) {return p==NULL;}

// Definitions for dsl_datacontainer functions

typedef void*                       dsl_type_t;
struct _dsl_datacontainer_t {
    dsl_container_t* container;
    dsl_type_t unit;
};
typedef struct _dsl_datacontainer_t dsl_datacontainer_t;

dsl_datacontainer_t* dsl_datacontainer_new            (dsl_type_t unit);
void		         dsl_datacontainer_delete         (dsl_datacontainer_t* datacontainer);
dsl_error_t          dsl_datacontainer_insert_level   (dsl_datacontainer_t* datacontainer, dsl_frequency_t frequency, dsl_level_t dB);
dsl_error_t      dsl_datacontainer_get_level_at_frequency    (const dsl_datacontainer_t* datacontainer, dsl_frequency_t frequency, dsl_level_t* OUTPUT);
dsl_type_t dsl_reSPL_unit();
dsl_type_t dsl_HL_unit();
dsl_type_t dsl_eHL_unit();
dsl_type_t dsl_nHL_unit();
dsl_type_t dsl_sfSPL_unit();
dsl_type_t dsl_NONE_unit();

dsl_type_t dsl_SPEECH_signal();
dsl_type_t dsl_COMPOSITE_signal();
dsl_type_t dsl_PURETONE_signal();

  dsl_error_t dsl_set_path (const char* path);
dsl_datacontainer_t* dsl_datacontainer_null();
int is_datacontainer_null(const dsl_datacontainer_t* p) {return p==NULL;}

// Definitions for dsl_dslmio functions

typedef struct _dsl_assessment_t dsl_assessment_t;
typedef struct _dsl_dslmio_t dsl_dslmio_t;

typedef unsigned short  dsl_month_t;
typedef enum {
    DSL_INSERT_TIP = 0x0,   ///< Insert earphone, with a TIP coupling to ear (Required)
    DSL_INSERT_EARMOLD,     ///< Insert earphone, with a Earmold coupling to ear (Required)
    DSL_ABR_TIP,            ///< Audiotory Brainstem Response, with a TIP coupling to ear (Required)
    DSL_ABR_EARMOLD,        ///< Audiotory Brainstem Response, with a Earmold coupling to ear
    DSL_TDH,                ///< TDH style headphone (Required)
    DSL_LS0,                ///< Soundfield at 0 degree azimuth
    DSL_LS45,               ///< Soundfield at 45 degree azimuth (Required)
    DSL_LS90,               ///< Soundfield at 90 degree azimuth
    DSL_RE_SPL,             ///< Direct measurement of threshold in the real ear in dB SPL
#ifdef ENABLE_DSLBC
    DSL_BAHD		    ///< Bone Anchored Hearing System
#endif
} dsl_transducer_type_t;
typedef enum {
    DSL_RECD = 0x0,         ///< Real Ear coupler difference
    DSL_REDD,               ///< Real Ear dial difference
    DSL_REUG,               ///< Real Ear Unaided Gain
    DSL_DIRECT_RECD,        ///< Direct Real Ear coupler difference measurement using hearing instrument as transducer
#ifdef ENABLE_DSLBC
    DSL_RHCD		    ///< Real Head Skull Simulator
#endif
} dsl_acoustic_transform_type_t;
typedef enum {
    DSL_DEFAULT = 0x0,      ///< use the Default procedure for the selected transform and transducer
    DSL_HA1_TIP,            ///< HA1 Foamtip used with RECD transforms
    DSL_HA2_TIP,            ///< HA2 Foamtip used with RECD transforms (Required)
    DSL_HA1_MOLD,           ///< HA1 Earmold used with RECD transforms (Required)
    DSL_HA2_MOLD,           ///< HA2 Earmold used with RECD transforms
    DSL_SF0,                ///< Soundfield at zero azimuth (mostly used with LS0 transducers)
    DSL_SF45,               ///< Soundfield at 45 azimuth (mostly used with LS45 transducers)
    DSL_SF90                ///< Soundfield at 90 azimuth (mostly used with LS90 transducers)
} dsl_acoustic_transform_procedures_type_t;
typedef struct _dsl_input_signal_t dsl_input_signal_t;
typedef enum {
    DSL_QUIET = 0x0,        ///< Quiet environment
    DSL_NOISE               ///< Noisy environment
} dsl_program_type_t;
typedef enum {
    DSL_Pediatric = 0x0,    ///< Pediatric targets will be generated
    DSL_Adults = 0x1        ///< Adult targets will be generated
} dsl_client_type_t;
typedef enum {
    DSL_MONAURAL = 0x0,     ///< Hearing instrument is worn on one ear only
    DSL_BINAURAL            ///< Hearing instruments are worn on each ear
} dsl_binaural_t;
typedef enum {
    DSL_LINEAR = 0x0,       ///< Linear hearing instruments
    DSL_WDRC                ///< Wide dynamic range compression instruments
} dsl_circuit_type_t;
typedef enum {
    DSL_REAR = 0x0,         ///< Real ear aided response
    DSL_REAG,               ///< Real ear aided gain
    DSL_REIG,               ///< Real ear insertion gain
    DSL_COUPLER_GAIN,       ///< 2cc coupler gain
    DSL_COUPLER_SPL         ///< 2cc coupler SPL
} dsl_verification_format_t;
typedef enum {
    DSL_BTE = 0x0,          ///< Behind the ear hearing instrument
    DSL_ITC,                ///< In the canal hearing instrument
    DSL_ITE,                ///< In the ear hearing instrument
    DSL_BODY,               ///< Body style hearing instrument
    DSL_CIC_DEEP,           ///< Deeply inserted completely in the canal
    DSL_CIC_SHALLOW,        ///< Completely in the canal
    DSL_RITE_DEEP,
    DSL_RITE_SHALLOW,
#ifdef ENABLE_DSLBC
    DSL_BAHD_ONBODY,        ///< Body worn bone anchored hearing instrument
    DSL_BAHD_ONHEAD         ///< OnHead bone anchored hearing instrument
#endif
} dsl_hearing_aid_style_t;
typedef struct _dsl_venting_correction_t dsl_venting_correction_t;
typedef dsl_type_t dsl_signal_t;

dsl_venting_correction_t * dsl_venting_correction_null();
dsl_input_signal_t* dsl_input_signal_new (dsl_signal_t type);
void                dsl_input_signal_delete (dsl_input_signal_t* signal);
dsl_error_t dsl_input_signal_spectra_with_overall_level (dsl_input_signal_t* signal, dsl_month_t client_age, dsl_level_t level);
dsl_datacontainer_t* dsl_input_signal_get_real_ear_levels   (const dsl_input_signal_t* signal);
dsl_datacontainer_t* dsl_input_signal_get_soundfield_levels (const dsl_input_signal_t* signal);
dsl_error_t dsl_input_signal_user_define_spectra (dsl_input_signal_t* signal, dsl_container_t* container);


void                 dsl_assessment_delete (dsl_assessment_t* assessment);
void                 dsl_dslmio_delete (dsl_dslmio_t* target);
dsl_assessment_t*    dsl_define_assessment (dsl_month_t age,
                                            dsl_transducer_type_t transducer,
                                            const dsl_datacontainer_t* th,
                                            const dsl_datacontainer_t* dc_1,
                                            const dsl_datacontainer_t* dc_2);
dsl_error_t          dsl_define_transform  (dsl_assessment_t* assessment,
                                            const dsl_container_t* data,
                                            dsl_acoustic_transform_type_t transform,
                                            dsl_acoustic_transform_procedures_type_t type);
dsl_dslmio_t*        dsl_generate_targets (const dsl_assessment_t* assessment,
                                           const dsl_input_signal_t* input,
                                           dsl_program_type_t program,
                                           dsl_client_type_t client_type,
                                           dsl_binaural_t binaural,
                                           dsl_circuit_type_t circuit,
                                           const dsl_channel_array_t* channels,
                                           const dsl_container_t* expanssion_threshold,
                                           const dsl_container_t* compression_threshold);
dsl_datacontainer_t* dsl_report_targets     (dsl_assessment_t* assessment,
                                             dsl_dslmio_t* target,
                                             dsl_verification_format_t format,
                                             dsl_hearing_aid_style_t ha_style,
                                             dsl_venting_correction_t* venting);

int dsl_number_of_all_valid_frequencies();
dsl_frequency_t dsl_get_valid_frequency(int i);
