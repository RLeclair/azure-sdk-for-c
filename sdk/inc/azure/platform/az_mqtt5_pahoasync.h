// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _az_MQTT5_PAHOASYNC_H
#define _az_MQTT5_PAHOASYNC_H

#include "MQTTAsync.h"
#include <azure/core/internal/az_mqtt5_internal.h>

#include <azure/core/_az_cfg_prefix.h>

typedef struct
{
  /**
   * @brief Platform options that are common across all MQTT 5 implementations.
   */
  az_mqtt5_options_common platform_options;

  /**
   * @brief The CA Trusted Roots span interpretable by the underlying MQTT 5 implementation.
   *
   * @details We recommend avoiding configuring this option and instead using the default
   * OpenSSL trusted roots instead.
   */
  az_span certificate_authority_trusted_roots;

  /**
   * @brief OpenSSL engine to use for the underlying MQTT 5 implementation.
   */
  az_span openssl_engine;

  /**
   * @brief Whether to use TLS for the underlying MQTT 5 implementation.
   */
  bool disable_tls;
} az_mqtt5_options;

struct az_mqtt5
{
  struct
  {
    /**
     * @brief Handle to the underlying MQTT 5 implementation (Mosquitto).
     */
    MQTTAsync* pahoasync_handle;

    /**
     * @brief Platform MQTT 5 client that is common across all MQTT 5 implementations.
     */
    az_mqtt5_common platform_mqtt5;

    /**
     * @brief MQTT 5 options for Mosquitto.
     */
    az_mqtt5_options options;
  } _internal;
};

typedef struct
{
  MQTTProperties* pahoasync_properties;
  MQTTProperty pahoasync_property; // TODO_L: Needed for copying
} az_mqtt5_property_bag;

/**
 * @brief MQTT 5 property string.
 *
 * @note String should always be freed using #az_mqtt5_property_free_string.
 */
typedef struct
{
  /**
   * @brief The string value of the property.
   */
  az_span str;
} az_mqtt5_property_string;

/**
 * @brief MQTT 5 property string pair.
 *
 * @note String should always be freed using #az_mqtt5_property_free_stringpair.
 */
typedef struct
{
  /**
   * @brief The key of the property.
   */
  az_span key;

  /**
   * @brief The value of the property.
   */
  az_span value;
} az_mqtt5_property_stringpair;

/**
 * @brief MQTT 5 property binary data.
 *
 * @note Binary data should always be freed using #az_mqtt5_property_free_binarydata.
 */
typedef struct
{
  /**
   * @brief The binary data value of the property.
   */
  az_span bindata;
} az_mqtt5_property_binarydata;

/**
 * @brief Initializes the MQTT 5 instance specific to Paho Async. //TODO_L: Who owns paho async?
 *
 * @param mqtt5 The MQTT 5 instance.
 * @param pahoasync_handle The Paho Async handle, can't be NULL.
 * @param options The MQTT 5 options.
 *
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result
az_mqtt5_init(az_mqtt5* mqtt5, MQTTAsync* pahoasync_handle, az_mqtt5_options const* options);

/**
 * @brief Initializes an MQTT 5 property bag instance specific to Paho.
 *
 * @param property_bag The MQTT 5 property bag instance.
 * @param mqtt5 The MQTT 5 instance.
 * @param pahoasync_properties The MQTT 5 paho property structure.
 *
 * @note For certain MQTT stacks, the property bag will need to be associated with a particular MQTT
 * client handle.
 *
 * @note Application is responsible for freeing any allocated memory for the property bag.
 * Lifetime of the property bag is tied to the lifetime of the MQTT 5 request, a property bag
 * can be reused by resetting the property bag using #az_mqtt5_property_bag_clear.
 *
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result az_mqtt5_property_bag_init(
    az_mqtt5_property_bag* property_bag,
    az_mqtt5* mqtt5,
    MQTTProperties* pahoasync_properties);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_MQTT5_PAHOASYNC_H
