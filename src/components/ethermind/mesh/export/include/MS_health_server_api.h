/**
    \file MS_health_server_api.h

    \brief This file defines the Mesh Health Model Application Interface
    - includes Data Structures and Methods for both Server and Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_MS_HEALTH_SERVER_API_
#define _H_MS_HEALTH_SERVER_API_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"


/* --------------------------------------------- Global Definitions */
/**
    \defgroup health_fault_values Fault Values
    \{
    This section lists the Fault Values defined for Health Model.
*/

/** Health Model - Fault Values */
/** No Fault */
#define MS_HEALTH_FAULT_NO_FAULT                                0x00
/** Battery Low Warning */
#define MS_HEALTH_FAULT_BATTERY_LOW_WARNING                     0x01
/** Battery Low Error */
#define MS_HEALTH_FAULT_BATTERY_LOW_ERROR                       0x02
/** Supply Voltage Too Low Warning */
#define MS_HEALTH_FAULT_SUPPLY_VOLTAGE_TOO_LOW_WARNING          0x03
/** Supply Voltage Too Low Error */
#define MS_HEALTH_FAULT_SUPPLY_VOLTAGE_TOO_LOW_ERROR            0x04
/** Supply Voltage Too High Warning */
#define MS_HEALTH_FAULT_SUPPLY_VOLTAGE_TOO_HIGH_WARNING         0x05
/** Supply Voltage Too High Error */
#define MS_HEALTH_FAULT_SUPPLY_VOLTAGE_TOO_HIGH_ERROR           0x06
/** Power Supply Interrupted Warning */
#define MS_HEALTH_FAULT_POWER_SUPPLY_INTERRUPTED_WARNING        0x07
/** Power Supply Interrupted Error */
#define MS_HEALTH_FAULT_POWER_SUPPLY_INTERRUPTED_ERROR          0x08
/** No Load Warning */
#define MS_HEALTH_FAULT_NO_LOAD_WARNING                         0x09
/** No Load Error */
#define MS_HEALTH_FAULT_NO_LOAD_ERROR                           0x0A
/** Overload Warning */
#define MS_HEALTH_FAULT_OVERLOAD_WARNING                        0x0B
/** Overload Error */
#define MS_HEALTH_FAULT_OVERLOAD_ERROR                          0x0C
/** Overheat Warning */
#define MS_HEALTH_FAULT_OVERHEAT_WARNING                        0x0D
/** Overheat Error */
#define MS_HEALTH_FAULT_OVERHEAT_ERROR                          0x0E
/** Condensation Warning */
#define MS_HEALTH_FAULT_CONDENSATION_WARNING                    0x0F
/** Condensation Error */
#define MS_HEALTH_FAULT_CONDENSATION_ERROR                      0x10
/** Vibration Warning */
#define MS_HEALTH_FAULT_VIBRATION_WARNING                       0x11
/** Vibration Error */
#define MS_HEALTH_FAULT_VIBRATION_ERROR                         0x12
/** Configuration Warning */
#define MS_HEALTH_FAULT_CONFIGURATION_WARNING                   0x13
/** Configuration Error */
#define MS_HEALTH_FAULT_CONFIGURATION_ERROR                     0x14
/** Element Not Calibrated Warning */
#define MS_HEALTH_FAULT_ELEMENT_NOT_CALIBRATED_WARNING          0x15
/** Element Not Calibrated Error */
#define MS_HEALTH_FAULT_ELEMENT_NOT_CALIBRATED_ERROR            0x16
/** Memory Warning */
#define MS_HEALTH_FAULT_MEMORY_WARNING                          0x17
/** Memory Error */
#define MS_HEALTH_FAULT_MEMORY_ERROR                            0x18
/** Self-Test Warning */
#define MS_HEALTH_FAULT_SELF_TEST_WARNING                       0x19
/** Self-Test Error */
#define MS_HEALTH_FAULT_SELF_TEST_ERROR                         0x1A
/** Input Too Low Warning */
#define MS_HEALTH_FAULT_INPUT_TOO_LOW_WARNING                   0x1B
/** Input Too Low Error */
#define MS_HEALTH_FAULT_INPUT_TOO_LOW_ERROR                     0x1C
/** Input Too High Warning */
#define MS_HEALTH_FAULT_INPUT_TOO_HIGH_WARNING                  0x1D
/** Input Too High Error */
#define MS_HEALTH_FAULT_INPUT_TOO_HIGH_ERROR                    0x1E
/** Input No Change Warning */
#define MS_HEALTH_FAULT_INPUT_NO_CHANGE_WARNING                 0x1F
/** Input No Change Error */
#define MS_HEALTH_FAULT_INPUT_NO_CHANGE_ERROR                   0x20
/** Actuator Blocked Warning */
#define MS_HEALTH_FAULT_ACTUATOR_BLOCKED_WARNING                0x21
/** Actuator Blocked Error */
#define MS_HEALTH_FAULT_ACTUATOR_BLOCKED_ERROR                  0x22
/** Housing Opened Warning */
#define MS_HEALTH_FAULT_HOUSING_OPENED_WARNING                  0x23
/** Housing Opened Error */
#define MS_HEALTH_FAULT_HOUSING_OPENED_ERROR                    0x24
/** Tamper Warning */
#define MS_HEALTH_FAULT_TAMPER_WARNING                          0x25
/** Tamper Error */
#define MS_HEALTH_FAULT_TAMPER_ERROR                            0x26
/** Device Moved Warning */
#define MS_HEALTH_FAULT_DEVICE_MOVED_WARNING                    0x27
/** Device Moved Error */
#define MS_HEALTH_FAULT_DEVICE_MOVED_ERROR                      0x28
/** Device Dropped Warning */
#define MS_HEALTH_FAULT_DEVICE_DROPPED_WARNING                  0x29
/** Device Dropped Error */
#define MS_HEALTH_FAULT_DEVICE_DROPPED_ERROR                    0x2A
/** Overflow Warning */
#define MS_HEALTH_FAULT_OVERFLOW_WARNING                        0x2B
/** Overflow Error */
#define MS_HEALTH_FAULT_OVERFLOW_ERROR                          0x2C
/** Empty Warning */
#define MS_HEALTH_FAULT_EMPTY_WARNING                           0x2D
/** Empty Error */
#define MS_HEALTH_FAULT_EMPTY_ERROR                             0x2E
/** Internal Bus Warning */
#define MS_HEALTH_FAULT_INTERNAL_BUS_WARNING                    0x2F
/** Internal Bus Error */
#define MS_HEALTH_FAULT_INTERNAL_BUS_ERROR                      0x30
/** Mechanism Jammed Warning */
#define MS_HEALTH_FAULT_MECHANISM_JAMMED_WARNING                0x31
/** Mechanism Jammed Error*/
#define MS_HEALTH_FAULT_MECHANISM_JAMMED_ERROR                  0x32

/* 0x33 - 0x7F: Reserved for Future Use */
/* 0x80 - 0xFF: Vendor Specific Warning / Error */

/** \} */

/**
    \defgroup health_server_events Health Server Events
    \{
    This section lists the Application Events defined for Health Server Model.
*/

/** Attention Start */
#define MS_HEALTH_SERVER_ATTENTION_START                       0x01

/** Attention Restart */
#define MS_HEALTH_SERVER_ATTENTION_RESTART                     0x02

/** Attention Stop */
#define MS_HEALTH_SERVER_ATTENTION_STOP                        0x03

/** \} */


/* --------------------------------------------- Data Types/ Structures */
/**
    \brief Health Server application Asynchronous Notification Callback.

    \par Description
    Health Server calls the registered callback to indicate events occurred to the
    application.

    \param handle        Model Handle.
    \param event_type    Health Server Event type.
    \param event_param   Parameter associated with the event if any or NULL.
    \param param_len     Size of the event parameter data. 0 if event param is NULL.
*/
typedef API_RESULT (* MS_HEALTH_SERVER_CB)
(
    MS_ACCESS_MODEL_HANDLE* handle,
    UINT8                    event_type,
    UINT8*                   event_param,
    UINT16                   param_len
) DECL_REENTRANT;


/**
    Health Server Self Test Function.
*/
/* TODO: Add context of the Health Server, so that associated current/registered fault can be updated */
typedef void (* MS_HEALTH_SERVER_SELF_TEST_FN)(UINT8 test_id, UINT16 company_id);

/**
    Health Server Self Test Funtion Structure.
*/
typedef struct _MS_HEALTH_SERVER_SELF_TEST
{
    /** Test ID */
    UINT8                           test_id;

    /** Self Test Function */
    MS_HEALTH_SERVER_SELF_TEST_FN    self_test_fn;

} MS_HEALTH_SERVER_SELF_TEST;


/* --------------------------------------------- Function */
/**
    \brief API to initialize Health Server model

    \par Description
    This is to initialize Health Server model and to register with Acess layer.

    \param [in] element_handle
                Element identifier to be associated with the model instance.

    \param [in, out] model_handle
                     Model identifier associated with the model instance on successful initialization.
                     After power cycle of an already provisioned node, the model handle will have
                     valid value and the same will be reused for registration.

    \param [in]      company_id
                     Company Identifier

    \param [in]      self_tests
                     List of Self Tests that can be run.

    \param [in]      num_self_tests
                     Number of Self Tests in the list.

    \param [in] appl_cb    Application Callback to be used by the Health Server.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_health_server_init
(
    /* IN */    MS_ACCESS_ELEMENT_HANDLE     element_handle,
    /* INOUT */ MS_ACCESS_MODEL_HANDLE*      model_handle,
    /* IN */    UINT16                       company_id,
    /* IN */    MS_HEALTH_SERVER_SELF_TEST* self_tests,
    /* IN */    UINT32                       num_self_tests,
    /* IN */    MS_HEALTH_SERVER_CB          appl_cb
);

/**
    \brief API to report self-test fault

    \par Description
    This is to report fault observed during self-test procedure.

    \param [in] model_handle
                Model Handle identifying the Health Server model instance.

    \param [in]      test_id
                     Identifier of the self-test

    \param [in]      company_id
                     Company Identifier

    \param [in]      fault_code
                     Fault value indicating the error.

    \return API_SUCCESS or an error code indicating reason for failure
*/
API_RESULT MS_health_server_report_fault
(
    /* IN */ MS_ACCESS_MODEL_HANDLE*      model_handle,
    /* IN */ UINT8                        test_id,
    /* IN */ UINT16                       company_id,
    /* IN */ UINT8                        fault_code
);

/**
    \cond ignore_this Ignore this fundtion while generating doxygen document
*/

API_RESULT MS_health_server_publish_current_status
(
    UCHAR*     status,
    UINT16     length
);
/**
    \endcond
*/

#endif /*_H_MS_HEALTH_SERVER_API_ */
