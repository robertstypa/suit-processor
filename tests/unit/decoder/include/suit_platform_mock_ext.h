/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "suit_platform/cmock_suit_platform.h"
#include <complex_arg.h>

#ifndef _SUIT_PLATFORM_MOCK_EXT_H
#define _SUIT_PLATFORM_MOCK_EXT_H

/** @file suit_platform_mock_ext.h
 * @brief Provides additional CMock-like functions that assert on the zcbor string contents.
 *
 * The CMock provides two main prototypes for checking the set of arguments, passed to the mocked APIs:
 *  - __cmock_<API>_ExpectArgsAndReturn(args..): that compare directly the values with the expected arguments
 *  - __cmock_<API>_ExpectWithArrayAndReturn(args.., *args..): that compare pointer arguments against an expected, pointed memory contents
 *
 * In case of pointers to ZCBOR strings, defined as the following structure:
 * struct zcbor_string {
 *     const uint8_t *value;
 *     size_t len;
 * };
 *
 * both of them are insufficient:
 *  - __cmock_<API>_ExpectArgsAndReturn(args..): will assert on the pointer to the structure,
 *    which usually points to the local variable, thus the address is unpredictable.
 *  - __cmock_<API>_ExpectWithArrayAndReturn(args.., *args..): will correctly verify the `len` field,
 *    but instead of checking value, pointed by the `value` field, it will assert on the pointed address.
 *    In most cases, it will be predictable, because the SUIT module passes pointers to the manifest buffer,
 *    but this is not true for checking temporary buffers, for example the Signature1 authentication block.
 *
 * To overcome this limitation, a complex_arg queues are used. They allow to:
 *  - Store the expected argument value with a custom assertion function on the queue through push_complex_arg(..) API
 *  - Store the expected return value on the queue through push_retval_arg API
 *  - Pop the queue head and execute the assertion using the assert_complex_arg(..) API
 *
 * To mix complex and non-complex argument types, the regular __cmock_<API>_ExpectArgsAndReturn(args..)
 * functions are used, but for each complex argument:
 *  - The queue is populated
 *  - The __cmock_<API>_IgnoreArg_<complex_arg_name> API is called
 * In addition to that:
 *  - A callback for running queue-based assertions is registered
 *  - The return value, expected to be returned by callback is added to the queue
 *
 * The implementation for those extensions should:
 * - Define queues for storing complex arguments, one for each mock using COMPLEX_ARG_Q_DEFINE(..) API.
 * - Implement the registered callbacks.
 *
 * Note: The generation of the CMock functions is requested by calling cmock_handle(<module.h> <module_subdir>)
 *       CMake function and the auto-generated functions are placed inside the build/mocks/<module_subdir> directory.
 */


/** @brief assert if length and values of two ZCBOR string differ.
 *
 * @param p1  Pointer to the first ZCBOR string.
 * @param p2  Pointer to the second ZCBOR string.
 */
void assert_zcbor_string(void *p1, void *p2);

#define __cmock_suit_plat_check_digest_ExpectComplexArgsAndReturn(alg_id, digest, payload, cmock_retval) { \
	extern complex_arg_q_t __check_digest_callback_queue; \
	push_complex_arg(digest, assert_zcbor_string, __check_digest_callback_queue); \
	push_complex_arg(payload, assert_zcbor_string, __check_digest_callback_queue); \
	push_retval_arg(cmock_retval, __check_digest_callback_queue); \
	__cmock_suit_plat_check_digest_AddCallback(__check_digest_callback); \
	__cmock_suit_plat_check_digest_ExpectAndReturn(alg_id, digest, payload, cmock_retval); \
	__cmock_suit_plat_check_digest_IgnoreArg_digest(); \
	__cmock_suit_plat_check_digest_IgnoreArg_payload(); \
}
int __check_digest_callback(enum suit_cose_alg alg_id, struct zcbor_string *digest, struct zcbor_string *payload, int cmock_num_calls);

#define __cmock_suit_plat_authenticate_manifest_ExpectComplexArgsAndReturn(manifest_component_id, alg_id, key_id, signature, data, cmock_retval) { \
	extern complex_arg_q_t __authenticate_manifest_callback_queue; \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __authenticate_manifest_callback_queue); \
	push_complex_arg(key_id, assert_zcbor_string, __authenticate_manifest_callback_queue); \
	push_complex_arg(signature, assert_zcbor_string, __authenticate_manifest_callback_queue); \
	push_complex_arg(data, assert_zcbor_string, __authenticate_manifest_callback_queue); \
	push_retval_arg(cmock_retval, __authenticate_manifest_callback_queue); \
	__cmock_suit_plat_authenticate_manifest_AddCallback(__authenticate_manifest_callback); \
	__cmock_suit_plat_authenticate_manifest_ExpectAndReturn(manifest_component_id, alg_id, key_id, signature, data, cmock_retval); \
	__cmock_suit_plat_authenticate_manifest_IgnoreArg_key_id(); \
	__cmock_suit_plat_authenticate_manifest_IgnoreArg_signature(); \
	__cmock_suit_plat_authenticate_manifest_IgnoreArg_data(); \
	__cmock_suit_plat_authenticate_manifest_IgnoreArg_manifest_component_id(); \
}
int __authenticate_manifest_callback(struct zcbor_string *manifest_component_id, enum suit_cose_alg alg_id, struct zcbor_string *key_id, struct zcbor_string *signature, struct zcbor_string *data, int cmock_num_calls);

#define __cmock_suit_plat_create_component_handle_ExpectComplexArgsAndReturn(component_id, dependency, handle, cmock_retval) { \
	extern complex_arg_q_t __get_component_handle_callback_queue; \
	push_complex_arg(component_id, assert_zcbor_string, __get_component_handle_callback_queue); \
	push_retval_arg(cmock_retval, __get_component_handle_callback_queue); \
	__cmock_suit_plat_create_component_handle_AddCallback(__get_component_handle_callback); \
	__cmock_suit_plat_create_component_handle_ExpectAndReturn(component_id, dependency, handle, cmock_retval); \
	__cmock_suit_plat_create_component_handle_IgnoreArg_component_id(); \
}
int __get_component_handle_callback(struct zcbor_string *component_id, bool dependency, suit_component_t *handle, int cmock_num_calls);

#define __cmock_suit_plat_check_image_match_ExpectComplexArgsAndReturn(image_handle, alg_id, digest, cmock_retval) { \
	extern complex_arg_q_t __check_image_match_callback_queue; \
	push_complex_arg(digest, assert_zcbor_string, __check_image_match_callback_queue); \
	push_retval_arg(cmock_retval, __check_image_match_callback_queue); \
	__cmock_suit_plat_check_image_match_AddCallback(__check_image_match_callback); \
	__cmock_suit_plat_check_image_match_ExpectAndReturn(image_handle, alg_id, digest, cmock_retval); \
	__cmock_suit_plat_check_image_match_IgnoreArg_digest(); \
}
int __check_image_match_callback(suit_component_t image_handle, enum suit_cose_alg alg_id, struct zcbor_string *digest, int cmock_num_calls);

#define __cmock_suit_plat_check_content_ExpectComplexArgsAndReturn(component_handle, content, cmock_retval) { \
	extern complex_arg_q_t __check_content_callback_queue; \
	push_complex_arg(content, assert_zcbor_string, __check_content_callback_queue); \
	push_retval_arg(cmock_retval, __check_content_callback_queue); \
	__cmock_suit_plat_check_content_AddCallback(__check_content_callback); \
	__cmock_suit_plat_check_content_ExpectAndReturn(component_handle, content, cmock_retval); \
	__cmock_suit_plat_check_content_IgnoreArg_content(); \
}
int __check_content_callback(suit_component_t component_handle, struct zcbor_string *content, int cmock_num_calls);

#define __cmock_suit_plat_check_vid_ExpectComplexArgsAndReturn(component_handle, vid_uuid, cmock_retval) { \
	extern complex_arg_q_t __check_vid_callback_queue; \
	push_complex_arg(vid_uuid, assert_zcbor_string, __check_vid_callback_queue); \
	push_retval_arg(cmock_retval, __check_vid_callback_queue); \
	__cmock_suit_plat_check_vid_AddCallback(__check_vid_callback); \
	__cmock_suit_plat_check_vid_ExpectAndReturn(component_handle, vid_uuid, cmock_retval); \
	__cmock_suit_plat_check_vid_IgnoreArg_vid_uuid(); \
}
int __check_vid_callback(suit_component_t component_handle, struct zcbor_string *vid_uuid, int cmock_num_calls);

#define __cmock_suit_plat_check_cid_ExpectComplexArgsAndReturn(component_handle, cid_uuid, cmock_retval) { \
	extern complex_arg_q_t __check_cid_callback_queue; \
	push_complex_arg(cid_uuid, assert_zcbor_string, __check_cid_callback_queue); \
	push_retval_arg(cmock_retval, __check_cid_callback_queue); \
	__cmock_suit_plat_check_cid_AddCallback(__check_cid_callback); \
	__cmock_suit_plat_check_cid_ExpectAndReturn(component_handle, cid_uuid, cmock_retval); \
	__cmock_suit_plat_check_cid_IgnoreArg_cid_uuid(); \
}
int __check_cid_callback(suit_component_t component_handle, struct zcbor_string *cid_uuid, int cmock_num_calls);

#define __cmock_suit_plat_check_did_ExpectComplexArgsAndReturn(component_handle, did_uuid, cmock_retval) { \
	extern complex_arg_q_t __check_did_callback_queue; \
	push_complex_arg(did_uuid, assert_zcbor_string, __check_did_callback_queue); \
	push_retval_arg(cmock_retval, __check_did_callback_queue); \
	__cmock_suit_plat_check_did_AddCallback(__check_did_callback); \
	__cmock_suit_plat_check_did_ExpectAndReturn(component_handle, did_uuid, cmock_retval); \
	__cmock_suit_plat_check_did_IgnoreArg_did_uuid(); \
}
int __check_did_callback(suit_component_t component_handle, struct zcbor_string *did_uuid, int cmock_num_calls);

#define __cmock_suit_plat_authorize_sequence_num_ExpectComplexArgsAndReturn(seq_name, manifest_component_id, seq_num, cmock_retval) { \
	extern complex_arg_q_t __authorize_sequence_num_callback_queue; \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __authorize_sequence_num_callback_queue); \
	push_retval_arg(cmock_retval, __authorize_sequence_num_callback_queue); \
	__cmock_suit_plat_authorize_sequence_num_AddCallback(__authorize_sequence_num_callback); \
	__cmock_suit_plat_authorize_sequence_num_ExpectAndReturn(seq_name, manifest_component_id, seq_num, cmock_retval); \
	__cmock_suit_plat_authorize_sequence_num_IgnoreArg_manifest_component_id(); \
}
int __authorize_sequence_num_callback(enum suit_command_sequence seq_name, struct zcbor_string *manifest_component_id, unsigned int seq_num, int cmock_num_calls);

#define __cmock_suit_plat_authorize_component_id_ExpectComplexArgsAndReturn(manifest_component_id, component_id, cmock_retval) { \
	extern complex_arg_q_t __authorize_component_id_callback_queue; \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __authorize_component_id_callback_queue); \
	push_complex_arg(component_id, assert_zcbor_string, __authorize_component_id_callback_queue); \
	push_retval_arg(cmock_retval, __authorize_component_id_callback_queue); \
	__cmock_suit_plat_authorize_component_id_AddCallback(__authorize_component_id_callback); \
	__cmock_suit_plat_authorize_component_id_ExpectAndReturn(manifest_component_id, component_id, cmock_retval); \
	__cmock_suit_plat_authorize_component_id_IgnoreArg_manifest_component_id(); \
	__cmock_suit_plat_authorize_component_id_IgnoreArg_component_id(); \
}
int __authorize_component_id_callback(struct zcbor_string *manifest_component_id, struct zcbor_string *component_id, int cmock_num_calls);

#define __cmock_suit_plat_check_fetch_ExpectComplexArgsAndReturn(dst_handle, uri, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __check_fetch_callback_queue; \
	push_complex_arg(uri, assert_zcbor_string, __check_fetch_callback_queue); \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __check_fetch_callback_queue); \
	push_retval_arg(cmock_retval, __check_fetch_callback_queue); \
	__cmock_suit_plat_check_fetch_AddCallback(__check_fetch_callback); \
	__cmock_suit_plat_check_fetch_ExpectAndReturn(dst_handle, uri, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_check_fetch_IgnoreArg_uri(); \
	__cmock_suit_plat_check_fetch_IgnoreArg_manifest_component_id(); \
}
int __check_fetch_callback(suit_component_t dst_handle, struct zcbor_string *uri, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);

#define __cmock_suit_plat_fetch_ExpectComplexArgsAndReturn(dst_handle, uri, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __fetch_callback_queue; \
	push_complex_arg(uri, assert_zcbor_string, __fetch_callback_queue); \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __fetch_callback_queue); \
	push_retval_arg(cmock_retval, __fetch_callback_queue); \
	__cmock_suit_plat_fetch_AddCallback(__fetch_callback); \
	__cmock_suit_plat_fetch_ExpectAndReturn(dst_handle, uri, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_fetch_IgnoreArg_uri(); \
	__cmock_suit_plat_fetch_IgnoreArg_manifest_component_id(); \
}
int __fetch_callback(suit_component_t dst_handle, struct zcbor_string *uri, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);

#define __cmock_suit_plat_check_fetch_integrated_ExpectComplexArgsAndReturn(dst_handle, payload, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __check_fetch_integrated_callback_queue; \
	push_complex_arg(payload, assert_zcbor_string, __check_fetch_integrated_callback_queue); \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __check_fetch_integrated_callback_queue); \
	push_retval_arg(cmock_retval, __check_fetch_integrated_callback_queue); \
	__cmock_suit_plat_check_fetch_integrated_AddCallback(__check_fetch_integrated_callback); \
	__cmock_suit_plat_check_fetch_integrated_ExpectAndReturn(dst_handle, payload, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_check_fetch_integrated_IgnoreArg_payload(); \
	__cmock_suit_plat_check_fetch_IgnoreArg_manifest_component_id(); \
}
int __check_fetch_integrated_callback(suit_component_t dst_handle, struct zcbor_string *payload, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);

#define __cmock_suit_plat_fetch_integrated_ExpectComplexArgsAndReturn(dst_handle, payload, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __fetch_integrated_callback_queue; \
	push_complex_arg(payload, assert_zcbor_string, __fetch_integrated_callback_queue); \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __fetch_integrated_callback_queue); \
	push_retval_arg(cmock_retval, __fetch_integrated_callback_queue); \
	__cmock_suit_plat_fetch_integrated_AddCallback(__fetch_integrated_callback); \
	__cmock_suit_plat_fetch_integrated_ExpectAndReturn(dst_handle, payload, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_fetch_integrated_IgnoreArg_payload(); \
	__cmock_suit_plat_fetch_integrated_IgnoreArg_manifest_component_id(); \
}
int __fetch_integrated_callback(suit_component_t dst_handle, struct zcbor_string *payload, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);

#define __cmock_suit_plat_check_copy_ExpectComplexArgsAndReturn(dst_handle, src_handle, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __check_copy_callback_queue; \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __check_copy_callback_queue); \
	push_retval_arg(cmock_retval, __check_copy_callback_queue); \
	__cmock_suit_plat_check_copy_AddCallback(__check_copy_callback); \
	__cmock_suit_plat_check_copy_ExpectAndReturn(dst_handle, src_handle, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_check_copy_IgnoreArg_manifest_component_id(); \
}
int __check_copy_callback(suit_component_t dst_handle, suit_component_t src_handle, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);

#define __cmock_suit_plat_copy_ExpectComplexArgsAndReturn(dst_handle, src_handle, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __copy_callback_queue; \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __copy_callback_queue); \
	push_retval_arg(cmock_retval, __copy_callback_queue); \
	__cmock_suit_plat_copy_AddCallback(__copy_callback); \
	__cmock_suit_plat_copy_ExpectAndReturn(dst_handle, src_handle, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_copy_IgnoreArg_manifest_component_id(); \
}
int __copy_callback(suit_component_t dst_handle, suit_component_t src_handle, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);


#define __cmock_suit_plat_sequence_completed_ExpectComplexArgsAndReturn(seq_name, manifest_component_id, envelope_str, envelope_len, cmock_retval) { \
	extern complex_arg_q_t __sequence_completed_callback_queue; \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __sequence_completed_callback_queue); \
	push_retval_arg(cmock_retval, __sequence_completed_callback_queue); \
	__cmock_suit_plat_sequence_completed_AddCallback(__sequence_completed_callback); \
	__cmock_suit_plat_sequence_completed_ExpectAndReturn(seq_name, manifest_component_id, envelope_str, envelope_len, cmock_retval); \
	__cmock_suit_plat_sequence_completed_IgnoreArg_manifest_component_id(); \
}
int __sequence_completed_callback(enum suit_command_sequence seq_name, struct zcbor_string *manifest_component_id, const uint8_t *envelope_str, size_t envelope_len, int cmock_num_calls);

#define __cmock_suit_plat_check_write_ExpectComplexArgsAndReturn(dst_handle, content, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __check_write_callback_queue; \
	push_complex_arg(content, assert_zcbor_string, __check_write_callback_queue); \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __check_write_callback_queue); \
	push_retval_arg(cmock_retval, __check_write_callback_queue); \
	__cmock_suit_plat_check_write_AddCallback(__check_write_callback); \
	__cmock_suit_plat_check_write_ExpectAndReturn(dst_handle, content, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_check_write_IgnoreArg_content(); \
	__cmock_suit_plat_check_write_IgnoreArg_manifest_component_id(); \
}
int __check_write_callback(suit_component_t dst_handle, struct zcbor_string *content, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);

#define __cmock_suit_plat_write_ExpectComplexArgsAndReturn(dst_handle, content, manifest_component_id, enc_info, cmock_retval) { \
	extern complex_arg_q_t __write_callback_queue; \
	push_complex_arg(content, assert_zcbor_string, __write_callback_queue); \
	push_complex_arg(manifest_component_id, assert_zcbor_string, __write_callback_queue); \
	push_retval_arg(cmock_retval, __write_callback_queue); \
	__cmock_suit_plat_write_AddCallback(__write_callback); \
	__cmock_suit_plat_write_ExpectAndReturn(dst_handle, content, manifest_component_id, enc_info, cmock_retval); \
	__cmock_suit_plat_write_IgnoreArg_content(); \
	__cmock_suit_plat_write_IgnoreArg_manifest_component_id(); \
}
int __write_callback(suit_component_t dst_handle, struct zcbor_string *content, struct zcbor_string *manifest_component_id, struct suit_encryption_info *enc_info, int cmock_num_calls);

#define __cmock_suit_plat_authorize_process_dependency_ExpectComplexArgsAndReturn(parent_component_id, child_component_id, seq_name, cmock_retval) { \
	extern complex_arg_q_t __dependency_seq_authorize_callback_queue; \
	push_complex_arg(parent_component_id, assert_zcbor_string, __dependency_seq_authorize_callback_queue); \
	push_complex_arg(child_component_id, assert_zcbor_string, __dependency_seq_authorize_callback_queue); \
	push_retval_arg(cmock_retval, __dependency_seq_authorize_callback_queue); \
	__cmock_suit_plat_authorize_process_dependency_AddCallback(__dependency_seq_authorize_callback); \
	__cmock_suit_plat_authorize_process_dependency_ExpectAndReturn(parent_component_id, child_component_id, seq_name, cmock_retval); \
	__cmock_suit_plat_authorize_process_dependency_IgnoreArg_parent_component_id(); \
	__cmock_suit_plat_authorize_process_dependency_IgnoreArg_child_component_id(); \
}
int __dependency_seq_authorize_callback(struct zcbor_string *parent_component_id, struct zcbor_string *child_component_id, enum suit_command_sequence seq_name, int cmock_num_calls);

#endif /* _SUIT_PLATFORM_MOCK_EXT_H */
