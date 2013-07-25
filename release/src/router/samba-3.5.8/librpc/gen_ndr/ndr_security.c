/* parser auto-generated by pidl */

#include "includes.h"
#include "../librpc/gen_ndr/ndr_security.h"

#include "librpc/gen_ndr/ndr_misc.h"
_PUBLIC_ enum ndr_err_code ndr_push_security_ace_flags(struct ndr_push *ndr, int ndr_flags, uint8_t r)
{
	NDR_CHECK(ndr_push_uint8(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_ace_flags(struct ndr_pull *ndr, int ndr_flags, uint8_t *r)
{
	uint8_t v;
	NDR_CHECK(ndr_pull_uint8(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace_flags(struct ndr_print *ndr, const char *name, uint8_t r)
{
	ndr_print_uint8(ndr, name, r);
	ndr->depth++;
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_OBJECT_INHERIT", SEC_ACE_FLAG_OBJECT_INHERIT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_CONTAINER_INHERIT", SEC_ACE_FLAG_CONTAINER_INHERIT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_NO_PROPAGATE_INHERIT", SEC_ACE_FLAG_NO_PROPAGATE_INHERIT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_INHERIT_ONLY", SEC_ACE_FLAG_INHERIT_ONLY, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_INHERITED_ACE", SEC_ACE_FLAG_INHERITED_ACE, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_VALID_INHERIT", SEC_ACE_FLAG_VALID_INHERIT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_SUCCESSFUL_ACCESS", SEC_ACE_FLAG_SUCCESSFUL_ACCESS, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint8_t), "SEC_ACE_FLAG_FAILED_ACCESS", SEC_ACE_FLAG_FAILED_ACCESS, r);
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_security_ace_type(struct ndr_push *ndr, int ndr_flags, enum security_ace_type r)
{
	NDR_CHECK(ndr_push_enum_uint8(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_ace_type(struct ndr_pull *ndr, int ndr_flags, enum security_ace_type *r)
{
	uint8_t v;
	NDR_CHECK(ndr_pull_enum_uint8(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace_type(struct ndr_print *ndr, const char *name, enum security_ace_type r)
{
	const char *val = NULL;

	switch (r) {
		case SEC_ACE_TYPE_ACCESS_ALLOWED: val = "SEC_ACE_TYPE_ACCESS_ALLOWED"; break;
		case SEC_ACE_TYPE_ACCESS_DENIED: val = "SEC_ACE_TYPE_ACCESS_DENIED"; break;
		case SEC_ACE_TYPE_SYSTEM_AUDIT: val = "SEC_ACE_TYPE_SYSTEM_AUDIT"; break;
		case SEC_ACE_TYPE_SYSTEM_ALARM: val = "SEC_ACE_TYPE_SYSTEM_ALARM"; break;
		case SEC_ACE_TYPE_ALLOWED_COMPOUND: val = "SEC_ACE_TYPE_ALLOWED_COMPOUND"; break;
		case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT: val = "SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT"; break;
		case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT: val = "SEC_ACE_TYPE_ACCESS_DENIED_OBJECT"; break;
		case SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT: val = "SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT"; break;
		case SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT: val = "SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT"; break;
	}
	ndr_print_enum(ndr, name, "ENUM", val, r);
}

static enum ndr_err_code ndr_push_security_ace_object_flags(struct ndr_push *ndr, int ndr_flags, uint32_t r)
{
	NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

static enum ndr_err_code ndr_pull_security_ace_object_flags(struct ndr_pull *ndr, int ndr_flags, uint32_t *r)
{
	uint32_t v;
	NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace_object_flags(struct ndr_print *ndr, const char *name, uint32_t r)
{
	ndr_print_uint32(ndr, name, r);
	ndr->depth++;
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SEC_ACE_OBJECT_TYPE_PRESENT", SEC_ACE_OBJECT_TYPE_PRESENT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT", SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT, r);
	ndr->depth--;
}

static enum ndr_err_code ndr_push_security_ace_object_type(struct ndr_push *ndr, int ndr_flags, const union security_ace_object_type *r)
{
	if (ndr_flags & NDR_SCALARS) {
		int level = ndr_push_get_switch_value(ndr, r);
		NDR_CHECK(ndr_push_union_align(ndr, 4));
		switch (level) {
			case SEC_ACE_OBJECT_TYPE_PRESENT: {
				NDR_CHECK(ndr_push_GUID(ndr, NDR_SCALARS, &r->type));
			break; }

			default: {
			break; }

		}
	}
	if (ndr_flags & NDR_BUFFERS) {
		int level = ndr_push_get_switch_value(ndr, r);
		switch (level) {
			case SEC_ACE_OBJECT_TYPE_PRESENT:
			break;

			default:
			break;

		}
	}
	return NDR_ERR_SUCCESS;
}

static enum ndr_err_code ndr_pull_security_ace_object_type(struct ndr_pull *ndr, int ndr_flags, union security_ace_object_type *r)
{
	int level;
	level = ndr_pull_get_switch_value(ndr, r);
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_union_align(ndr, 4));
		switch (level) {
			case SEC_ACE_OBJECT_TYPE_PRESENT: {
				NDR_CHECK(ndr_pull_GUID(ndr, NDR_SCALARS, &r->type));
			break; }

			default: {
			break; }

		}
	}
	if (ndr_flags & NDR_BUFFERS) {
		switch (level) {
			case SEC_ACE_OBJECT_TYPE_PRESENT:
			break;

			default:
			break;

		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace_object_type(struct ndr_print *ndr, const char *name, const union security_ace_object_type *r)
{
	int level;
	level = ndr_print_get_switch_value(ndr, r);
	ndr_print_union(ndr, name, level, "security_ace_object_type");
	switch (level) {
		case SEC_ACE_OBJECT_TYPE_PRESENT:
			ndr_print_GUID(ndr, "type", &r->type);
		break;

		default:
		break;

	}
}

static enum ndr_err_code ndr_push_security_ace_object_inherited_type(struct ndr_push *ndr, int ndr_flags, const union security_ace_object_inherited_type *r)
{
	if (ndr_flags & NDR_SCALARS) {
		int level = ndr_push_get_switch_value(ndr, r);
		NDR_CHECK(ndr_push_union_align(ndr, 4));
		switch (level) {
			case SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT: {
				NDR_CHECK(ndr_push_GUID(ndr, NDR_SCALARS, &r->inherited_type));
			break; }

			default: {
			break; }

		}
	}
	if (ndr_flags & NDR_BUFFERS) {
		int level = ndr_push_get_switch_value(ndr, r);
		switch (level) {
			case SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT:
			break;

			default:
			break;

		}
	}
	return NDR_ERR_SUCCESS;
}

static enum ndr_err_code ndr_pull_security_ace_object_inherited_type(struct ndr_pull *ndr, int ndr_flags, union security_ace_object_inherited_type *r)
{
	int level;
	level = ndr_pull_get_switch_value(ndr, r);
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_union_align(ndr, 4));
		switch (level) {
			case SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT: {
				NDR_CHECK(ndr_pull_GUID(ndr, NDR_SCALARS, &r->inherited_type));
			break; }

			default: {
			break; }

		}
	}
	if (ndr_flags & NDR_BUFFERS) {
		switch (level) {
			case SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT:
			break;

			default:
			break;

		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace_object_inherited_type(struct ndr_print *ndr, const char *name, const union security_ace_object_inherited_type *r)
{
	int level;
	level = ndr_print_get_switch_value(ndr, r);
	ndr_print_union(ndr, name, level, "security_ace_object_inherited_type");
	switch (level) {
		case SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT:
			ndr_print_GUID(ndr, "inherited_type", &r->inherited_type);
		break;

		default:
		break;

	}
}

static enum ndr_err_code ndr_push_security_ace_object(struct ndr_push *ndr, int ndr_flags, const struct security_ace_object *r)
{
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_push_align(ndr, 4));
		NDR_CHECK(ndr_push_security_ace_object_flags(ndr, NDR_SCALARS, r->flags));
		NDR_CHECK(ndr_push_set_switch_value(ndr, &r->type, r->flags & SEC_ACE_OBJECT_TYPE_PRESENT));
		NDR_CHECK(ndr_push_security_ace_object_type(ndr, NDR_SCALARS, &r->type));
		NDR_CHECK(ndr_push_set_switch_value(ndr, &r->inherited_type, r->flags & SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT));
		NDR_CHECK(ndr_push_security_ace_object_inherited_type(ndr, NDR_SCALARS, &r->inherited_type));
		NDR_CHECK(ndr_push_trailer_align(ndr, 4));
	}
	if (ndr_flags & NDR_BUFFERS) {
		NDR_CHECK(ndr_push_security_ace_object_type(ndr, NDR_BUFFERS, &r->type));
		NDR_CHECK(ndr_push_security_ace_object_inherited_type(ndr, NDR_BUFFERS, &r->inherited_type));
	}
	return NDR_ERR_SUCCESS;
}

static enum ndr_err_code ndr_pull_security_ace_object(struct ndr_pull *ndr, int ndr_flags, struct security_ace_object *r)
{
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_align(ndr, 4));
		NDR_CHECK(ndr_pull_security_ace_object_flags(ndr, NDR_SCALARS, &r->flags));
		NDR_CHECK(ndr_pull_set_switch_value(ndr, &r->type, r->flags & SEC_ACE_OBJECT_TYPE_PRESENT));
		NDR_CHECK(ndr_pull_security_ace_object_type(ndr, NDR_SCALARS, &r->type));
		NDR_CHECK(ndr_pull_set_switch_value(ndr, &r->inherited_type, r->flags & SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT));
		NDR_CHECK(ndr_pull_security_ace_object_inherited_type(ndr, NDR_SCALARS, &r->inherited_type));
		NDR_CHECK(ndr_pull_trailer_align(ndr, 4));
	}
	if (ndr_flags & NDR_BUFFERS) {
		NDR_CHECK(ndr_pull_security_ace_object_type(ndr, NDR_BUFFERS, &r->type));
		NDR_CHECK(ndr_pull_security_ace_object_inherited_type(ndr, NDR_BUFFERS, &r->inherited_type));
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace_object(struct ndr_print *ndr, const char *name, const struct security_ace_object *r)
{
	ndr_print_struct(ndr, name, "security_ace_object");
	ndr->depth++;
	ndr_print_security_ace_object_flags(ndr, "flags", r->flags);
	ndr_print_set_switch_value(ndr, &r->type, r->flags & SEC_ACE_OBJECT_TYPE_PRESENT);
	ndr_print_security_ace_object_type(ndr, "type", &r->type);
	ndr_print_set_switch_value(ndr, &r->inherited_type, r->flags & SEC_ACE_INHERITED_OBJECT_TYPE_PRESENT);
	ndr_print_security_ace_object_inherited_type(ndr, "inherited_type", &r->inherited_type);
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_security_ace_object_ctr(struct ndr_push *ndr, int ndr_flags, const union security_ace_object_ctr *r)
{
	if (ndr_flags & NDR_SCALARS) {
		int level = ndr_push_get_switch_value(ndr, r);
		NDR_CHECK(ndr_push_union_align(ndr, 4));
		switch (level) {
			case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT: {
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT: {
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			case SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT: {
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			case SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT: {
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			default: {
			break; }

		}
	}
	if (ndr_flags & NDR_BUFFERS) {
		int level = ndr_push_get_switch_value(ndr, r);
		switch (level) {
			case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT:
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT:
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			case SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT:
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			case SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT:
				NDR_CHECK(ndr_push_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			default:
			break;

		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_ace_object_ctr(struct ndr_pull *ndr, int ndr_flags, union security_ace_object_ctr *r)
{
	int level;
	level = ndr_pull_get_switch_value(ndr, r);
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_union_align(ndr, 4));
		switch (level) {
			case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT: {
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT: {
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			case SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT: {
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			case SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT: {
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_SCALARS, &r->object));
			break; }

			default: {
			break; }

		}
	}
	if (ndr_flags & NDR_BUFFERS) {
		switch (level) {
			case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT:
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT:
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			case SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT:
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			case SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT:
				NDR_CHECK(ndr_pull_security_ace_object(ndr, NDR_BUFFERS, &r->object));
			break;

			default:
			break;

		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace_object_ctr(struct ndr_print *ndr, const char *name, const union security_ace_object_ctr *r)
{
	int level;
	level = ndr_print_get_switch_value(ndr, r);
	ndr_print_union(ndr, name, level, "security_ace_object_ctr");
	switch (level) {
		case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT:
			ndr_print_security_ace_object(ndr, "object", &r->object);
		break;

		case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT:
			ndr_print_security_ace_object(ndr, "object", &r->object);
		break;

		case SEC_ACE_TYPE_SYSTEM_AUDIT_OBJECT:
			ndr_print_security_ace_object(ndr, "object", &r->object);
		break;

		case SEC_ACE_TYPE_SYSTEM_ALARM_OBJECT:
			ndr_print_security_ace_object(ndr, "object", &r->object);
		break;

		default:
		break;

	}
}

_PUBLIC_ enum ndr_err_code ndr_push_security_ace(struct ndr_push *ndr, int ndr_flags, const struct security_ace *r)
{
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_push_align(ndr, 4));
		NDR_CHECK(ndr_push_security_ace_type(ndr, NDR_SCALARS, r->type));
		NDR_CHECK(ndr_push_security_ace_flags(ndr, NDR_SCALARS, r->flags));
		NDR_CHECK(ndr_push_uint16(ndr, NDR_SCALARS, ndr_size_security_ace(r, ndr->iconv_convenience, ndr->flags)));
		NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r->access_mask));
		NDR_CHECK(ndr_push_set_switch_value(ndr, &r->object, r->type));
		NDR_CHECK(ndr_push_security_ace_object_ctr(ndr, NDR_SCALARS, &r->object));
		NDR_CHECK(ndr_push_dom_sid(ndr, NDR_SCALARS, &r->trustee));
		NDR_CHECK(ndr_push_trailer_align(ndr, 4));
	}
	if (ndr_flags & NDR_BUFFERS) {
		NDR_CHECK(ndr_push_security_ace_object_ctr(ndr, NDR_BUFFERS, &r->object));
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_ace(struct ndr_print *ndr, const char *name, const struct security_ace *r)
{
	ndr_print_struct(ndr, name, "security_ace");
	ndr->depth++;
	ndr_print_security_ace_type(ndr, "type", r->type);
	ndr_print_security_ace_flags(ndr, "flags", r->flags);
	ndr_print_uint16(ndr, "size", (ndr->flags & LIBNDR_PRINT_SET_VALUES)?ndr_size_security_ace(r, ndr->iconv_convenience, ndr->flags):r->size);
	ndr_print_uint32(ndr, "access_mask", r->access_mask);
	ndr_print_set_switch_value(ndr, &r->object, r->type);
	ndr_print_security_ace_object_ctr(ndr, "object", &r->object);
	ndr_print_dom_sid(ndr, "trustee", &r->trustee);
	ndr->depth--;
}

static enum ndr_err_code ndr_push_security_acl_revision(struct ndr_push *ndr, int ndr_flags, enum security_acl_revision r)
{
	NDR_CHECK(ndr_push_enum_uint1632(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

static enum ndr_err_code ndr_pull_security_acl_revision(struct ndr_pull *ndr, int ndr_flags, enum security_acl_revision *r)
{
	uint16_t v;
	NDR_CHECK(ndr_pull_enum_uint1632(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_acl_revision(struct ndr_print *ndr, const char *name, enum security_acl_revision r)
{
	const char *val = NULL;

	switch (r) {
		case SECURITY_ACL_REVISION_NT4: val = "SECURITY_ACL_REVISION_NT4"; break;
		case SECURITY_ACL_REVISION_ADS: val = "SECURITY_ACL_REVISION_ADS"; break;
	}
	ndr_print_enum(ndr, name, "ENUM", val, r);
}

_PUBLIC_ enum ndr_err_code ndr_push_security_acl(struct ndr_push *ndr, int ndr_flags, const struct security_acl *r)
{
	uint32_t cntr_aces_0;
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_push_align(ndr, 4));
		NDR_CHECK(ndr_push_security_acl_revision(ndr, NDR_SCALARS, r->revision));
		NDR_CHECK(ndr_push_uint16(ndr, NDR_SCALARS, ndr_size_security_acl(r, ndr->iconv_convenience, ndr->flags)));
		NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r->num_aces));
		for (cntr_aces_0 = 0; cntr_aces_0 < r->num_aces; cntr_aces_0++) {
			NDR_CHECK(ndr_push_security_ace(ndr, NDR_SCALARS, &r->aces[cntr_aces_0]));
		}
		NDR_CHECK(ndr_push_trailer_align(ndr, 4));
	}
	if (ndr_flags & NDR_BUFFERS) {
		for (cntr_aces_0 = 0; cntr_aces_0 < r->num_aces; cntr_aces_0++) {
			NDR_CHECK(ndr_push_security_ace(ndr, NDR_BUFFERS, &r->aces[cntr_aces_0]));
		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_acl(struct ndr_pull *ndr, int ndr_flags, struct security_acl *r)
{
	uint32_t cntr_aces_0;
	TALLOC_CTX *_mem_save_aces_0;
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_align(ndr, 4));
		NDR_CHECK(ndr_pull_security_acl_revision(ndr, NDR_SCALARS, &r->revision));
		NDR_CHECK(ndr_pull_uint16(ndr, NDR_SCALARS, &r->size));
		NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &r->num_aces));
		if (r->num_aces > 1000) {
			return ndr_pull_error(ndr, NDR_ERR_RANGE, "value out of range");
		}
		NDR_PULL_ALLOC_N(ndr, r->aces, r->num_aces);
		_mem_save_aces_0 = NDR_PULL_GET_MEM_CTX(ndr);
		NDR_PULL_SET_MEM_CTX(ndr, r->aces, 0);
		for (cntr_aces_0 = 0; cntr_aces_0 < r->num_aces; cntr_aces_0++) {
			NDR_CHECK(ndr_pull_security_ace(ndr, NDR_SCALARS, &r->aces[cntr_aces_0]));
		}
		NDR_PULL_SET_MEM_CTX(ndr, _mem_save_aces_0, 0);
		NDR_CHECK(ndr_pull_trailer_align(ndr, 4));
	}
	if (ndr_flags & NDR_BUFFERS) {
		_mem_save_aces_0 = NDR_PULL_GET_MEM_CTX(ndr);
		NDR_PULL_SET_MEM_CTX(ndr, r->aces, 0);
		for (cntr_aces_0 = 0; cntr_aces_0 < r->num_aces; cntr_aces_0++) {
			NDR_CHECK(ndr_pull_security_ace(ndr, NDR_BUFFERS, &r->aces[cntr_aces_0]));
		}
		NDR_PULL_SET_MEM_CTX(ndr, _mem_save_aces_0, 0);
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_acl(struct ndr_print *ndr, const char *name, const struct security_acl *r)
{
	uint32_t cntr_aces_0;
	ndr_print_struct(ndr, name, "security_acl");
	ndr->depth++;
	ndr_print_security_acl_revision(ndr, "revision", r->revision);
	ndr_print_uint16(ndr, "size", (ndr->flags & LIBNDR_PRINT_SET_VALUES)?ndr_size_security_acl(r, ndr->iconv_convenience, ndr->flags):r->size);
	ndr_print_uint32(ndr, "num_aces", r->num_aces);
	ndr->print(ndr, "%s: ARRAY(%d)", "aces", (int)r->num_aces);
	ndr->depth++;
	for (cntr_aces_0=0;cntr_aces_0<r->num_aces;cntr_aces_0++) {
		char *idx_0=NULL;
		if (asprintf(&idx_0, "[%d]", cntr_aces_0) != -1) {
			ndr_print_security_ace(ndr, "aces", &r->aces[cntr_aces_0]);
			free(idx_0);
		}
	}
	ndr->depth--;
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_security_descriptor_revision(struct ndr_push *ndr, int ndr_flags, enum security_descriptor_revision r)
{
	NDR_CHECK(ndr_push_enum_uint8(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_descriptor_revision(struct ndr_pull *ndr, int ndr_flags, enum security_descriptor_revision *r)
{
	uint8_t v;
	NDR_CHECK(ndr_pull_enum_uint8(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_descriptor_revision(struct ndr_print *ndr, const char *name, enum security_descriptor_revision r)
{
	const char *val = NULL;

	switch (r) {
		case SECURITY_DESCRIPTOR_REVISION_1: val = "SECURITY_DESCRIPTOR_REVISION_1"; break;
	}
	ndr_print_enum(ndr, name, "ENUM", val, r);
}

_PUBLIC_ enum ndr_err_code ndr_push_security_descriptor_type(struct ndr_push *ndr, int ndr_flags, uint16_t r)
{
	NDR_CHECK(ndr_push_uint16(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_descriptor_type(struct ndr_pull *ndr, int ndr_flags, uint16_t *r)
{
	uint16_t v;
	NDR_CHECK(ndr_pull_uint16(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_descriptor_type(struct ndr_print *ndr, const char *name, uint16_t r)
{
	ndr_print_uint16(ndr, name, r);
	ndr->depth++;
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_OWNER_DEFAULTED", SEC_DESC_OWNER_DEFAULTED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_GROUP_DEFAULTED", SEC_DESC_GROUP_DEFAULTED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_DACL_PRESENT", SEC_DESC_DACL_PRESENT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_DACL_DEFAULTED", SEC_DESC_DACL_DEFAULTED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_SACL_PRESENT", SEC_DESC_SACL_PRESENT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_SACL_DEFAULTED", SEC_DESC_SACL_DEFAULTED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_DACL_TRUSTED", SEC_DESC_DACL_TRUSTED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_SERVER_SECURITY", SEC_DESC_SERVER_SECURITY, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_DACL_AUTO_INHERIT_REQ", SEC_DESC_DACL_AUTO_INHERIT_REQ, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_SACL_AUTO_INHERIT_REQ", SEC_DESC_SACL_AUTO_INHERIT_REQ, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_DACL_AUTO_INHERITED", SEC_DESC_DACL_AUTO_INHERITED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_SACL_AUTO_INHERITED", SEC_DESC_SACL_AUTO_INHERITED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_DACL_PROTECTED", SEC_DESC_DACL_PROTECTED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_SACL_PROTECTED", SEC_DESC_SACL_PROTECTED, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_RM_CONTROL_VALID", SEC_DESC_RM_CONTROL_VALID, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint16_t), "SEC_DESC_SELF_RELATIVE", SEC_DESC_SELF_RELATIVE, r);
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_security_descriptor(struct ndr_push *ndr, int ndr_flags, const struct security_descriptor *r)
{
	{
		uint32_t _flags_save_STRUCT = ndr->flags;
		ndr_set_flags(&ndr->flags, LIBNDR_FLAG_LITTLE_ENDIAN);
		if (ndr_flags & NDR_SCALARS) {
			NDR_CHECK(ndr_push_align(ndr, 5));
			NDR_CHECK(ndr_push_security_descriptor_revision(ndr, NDR_SCALARS, r->revision));
			NDR_CHECK(ndr_push_security_descriptor_type(ndr, NDR_SCALARS, r->type));
			NDR_CHECK(ndr_push_relative_ptr1(ndr, r->owner_sid));
			NDR_CHECK(ndr_push_relative_ptr1(ndr, r->group_sid));
			NDR_CHECK(ndr_push_relative_ptr1(ndr, r->sacl));
			NDR_CHECK(ndr_push_relative_ptr1(ndr, r->dacl));
			NDR_CHECK(ndr_push_trailer_align(ndr, 5));
		}
		if (ndr_flags & NDR_BUFFERS) {
			if (r->owner_sid) {
				NDR_CHECK(ndr_push_relative_ptr2_start(ndr, r->owner_sid));
				NDR_CHECK(ndr_push_dom_sid(ndr, NDR_SCALARS, r->owner_sid));
				NDR_CHECK(ndr_push_relative_ptr2_end(ndr, r->owner_sid));
			}
			if (r->group_sid) {
				NDR_CHECK(ndr_push_relative_ptr2_start(ndr, r->group_sid));
				NDR_CHECK(ndr_push_dom_sid(ndr, NDR_SCALARS, r->group_sid));
				NDR_CHECK(ndr_push_relative_ptr2_end(ndr, r->group_sid));
			}
			if (r->sacl) {
				NDR_CHECK(ndr_push_relative_ptr2_start(ndr, r->sacl));
				NDR_CHECK(ndr_push_security_acl(ndr, NDR_SCALARS|NDR_BUFFERS, r->sacl));
				NDR_CHECK(ndr_push_relative_ptr2_end(ndr, r->sacl));
			}
			if (r->dacl) {
				NDR_CHECK(ndr_push_relative_ptr2_start(ndr, r->dacl));
				NDR_CHECK(ndr_push_security_acl(ndr, NDR_SCALARS|NDR_BUFFERS, r->dacl));
				NDR_CHECK(ndr_push_relative_ptr2_end(ndr, r->dacl));
			}
		}
		ndr->flags = _flags_save_STRUCT;
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_descriptor(struct ndr_pull *ndr, int ndr_flags, struct security_descriptor *r)
{
	uint32_t _ptr_owner_sid;
	TALLOC_CTX *_mem_save_owner_sid_0;
	uint32_t _ptr_group_sid;
	TALLOC_CTX *_mem_save_group_sid_0;
	uint32_t _ptr_sacl;
	TALLOC_CTX *_mem_save_sacl_0;
	uint32_t _ptr_dacl;
	TALLOC_CTX *_mem_save_dacl_0;
	{
		uint32_t _flags_save_STRUCT = ndr->flags;
		ndr_set_flags(&ndr->flags, LIBNDR_FLAG_LITTLE_ENDIAN);
		if (ndr_flags & NDR_SCALARS) {
			NDR_CHECK(ndr_pull_align(ndr, 5));
			NDR_CHECK(ndr_pull_security_descriptor_revision(ndr, NDR_SCALARS, &r->revision));
			NDR_CHECK(ndr_pull_security_descriptor_type(ndr, NDR_SCALARS, &r->type));
			NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_owner_sid));
			if (_ptr_owner_sid) {
				NDR_PULL_ALLOC(ndr, r->owner_sid);
				NDR_CHECK(ndr_pull_relative_ptr1(ndr, r->owner_sid, _ptr_owner_sid));
			} else {
				r->owner_sid = NULL;
			}
			NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_group_sid));
			if (_ptr_group_sid) {
				NDR_PULL_ALLOC(ndr, r->group_sid);
				NDR_CHECK(ndr_pull_relative_ptr1(ndr, r->group_sid, _ptr_group_sid));
			} else {
				r->group_sid = NULL;
			}
			NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_sacl));
			if (_ptr_sacl) {
				NDR_PULL_ALLOC(ndr, r->sacl);
				NDR_CHECK(ndr_pull_relative_ptr1(ndr, r->sacl, _ptr_sacl));
			} else {
				r->sacl = NULL;
			}
			NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_dacl));
			if (_ptr_dacl) {
				NDR_PULL_ALLOC(ndr, r->dacl);
				NDR_CHECK(ndr_pull_relative_ptr1(ndr, r->dacl, _ptr_dacl));
			} else {
				r->dacl = NULL;
			}
			NDR_CHECK(ndr_pull_trailer_align(ndr, 5));
		}
		if (ndr_flags & NDR_BUFFERS) {
			if (r->owner_sid) {
				uint32_t _relative_save_offset;
				_relative_save_offset = ndr->offset;
				NDR_CHECK(ndr_pull_relative_ptr2(ndr, r->owner_sid));
				_mem_save_owner_sid_0 = NDR_PULL_GET_MEM_CTX(ndr);
				NDR_PULL_SET_MEM_CTX(ndr, r->owner_sid, 0);
				NDR_CHECK(ndr_pull_dom_sid(ndr, NDR_SCALARS, r->owner_sid));
				NDR_PULL_SET_MEM_CTX(ndr, _mem_save_owner_sid_0, 0);
				ndr->offset = _relative_save_offset;
			}
			if (r->group_sid) {
				uint32_t _relative_save_offset;
				_relative_save_offset = ndr->offset;
				NDR_CHECK(ndr_pull_relative_ptr2(ndr, r->group_sid));
				_mem_save_group_sid_0 = NDR_PULL_GET_MEM_CTX(ndr);
				NDR_PULL_SET_MEM_CTX(ndr, r->group_sid, 0);
				NDR_CHECK(ndr_pull_dom_sid(ndr, NDR_SCALARS, r->group_sid));
				NDR_PULL_SET_MEM_CTX(ndr, _mem_save_group_sid_0, 0);
				ndr->offset = _relative_save_offset;
			}
			if (r->sacl) {
				uint32_t _relative_save_offset;
				_relative_save_offset = ndr->offset;
				NDR_CHECK(ndr_pull_relative_ptr2(ndr, r->sacl));
				_mem_save_sacl_0 = NDR_PULL_GET_MEM_CTX(ndr);
				NDR_PULL_SET_MEM_CTX(ndr, r->sacl, 0);
				NDR_CHECK(ndr_pull_security_acl(ndr, NDR_SCALARS|NDR_BUFFERS, r->sacl));
				NDR_PULL_SET_MEM_CTX(ndr, _mem_save_sacl_0, 0);
				ndr->offset = _relative_save_offset;
			}
			if (r->dacl) {
				uint32_t _relative_save_offset;
				_relative_save_offset = ndr->offset;
				NDR_CHECK(ndr_pull_relative_ptr2(ndr, r->dacl));
				_mem_save_dacl_0 = NDR_PULL_GET_MEM_CTX(ndr);
				NDR_PULL_SET_MEM_CTX(ndr, r->dacl, 0);
				NDR_CHECK(ndr_pull_security_acl(ndr, NDR_SCALARS|NDR_BUFFERS, r->dacl));
				NDR_PULL_SET_MEM_CTX(ndr, _mem_save_dacl_0, 0);
				ndr->offset = _relative_save_offset;
			}
		}
		ndr->flags = _flags_save_STRUCT;
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_descriptor(struct ndr_print *ndr, const char *name, const struct security_descriptor *r)
{
	ndr_print_struct(ndr, name, "security_descriptor");
	{
		uint32_t _flags_save_STRUCT = ndr->flags;
		ndr_set_flags(&ndr->flags, LIBNDR_FLAG_LITTLE_ENDIAN);
		ndr->depth++;
		ndr_print_security_descriptor_revision(ndr, "revision", r->revision);
		ndr_print_security_descriptor_type(ndr, "type", r->type);
		ndr_print_ptr(ndr, "owner_sid", r->owner_sid);
		ndr->depth++;
		if (r->owner_sid) {
			ndr_print_dom_sid(ndr, "owner_sid", r->owner_sid);
		}
		ndr->depth--;
		ndr_print_ptr(ndr, "group_sid", r->group_sid);
		ndr->depth++;
		if (r->group_sid) {
			ndr_print_dom_sid(ndr, "group_sid", r->group_sid);
		}
		ndr->depth--;
		ndr_print_ptr(ndr, "sacl", r->sacl);
		ndr->depth++;
		if (r->sacl) {
			ndr_print_security_acl(ndr, "sacl", r->sacl);
		}
		ndr->depth--;
		ndr_print_ptr(ndr, "dacl", r->dacl);
		ndr->depth++;
		if (r->dacl) {
			ndr_print_security_acl(ndr, "dacl", r->dacl);
		}
		ndr->depth--;
		ndr->depth--;
		ndr->flags = _flags_save_STRUCT;
	}
}

_PUBLIC_ enum ndr_err_code ndr_push_sec_desc_buf(struct ndr_push *ndr, int ndr_flags, const struct sec_desc_buf *r)
{
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_push_align(ndr, 5));
		NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, ndr_size_security_descriptor(r->sd, ndr->iconv_convenience, ndr->flags)));
		NDR_CHECK(ndr_push_unique_ptr(ndr, r->sd));
		NDR_CHECK(ndr_push_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		if (r->sd) {
			{
				struct ndr_push *_ndr_sd;
				NDR_CHECK(ndr_push_subcontext_start(ndr, &_ndr_sd, 4, -1));
				NDR_CHECK(ndr_push_security_descriptor(_ndr_sd, NDR_SCALARS|NDR_BUFFERS, r->sd));
				NDR_CHECK(ndr_push_subcontext_end(ndr, _ndr_sd, 4, -1));
			}
		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_sec_desc_buf(struct ndr_pull *ndr, int ndr_flags, struct sec_desc_buf *r)
{
	uint32_t _ptr_sd;
	TALLOC_CTX *_mem_save_sd_0;
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_align(ndr, 5));
		NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &r->sd_size));
		if (r->sd_size > 0x40000) {
			return ndr_pull_error(ndr, NDR_ERR_RANGE, "value out of range");
		}
		NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_sd));
		if (_ptr_sd) {
			NDR_PULL_ALLOC(ndr, r->sd);
		} else {
			r->sd = NULL;
		}
		NDR_CHECK(ndr_pull_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		if (r->sd) {
			_mem_save_sd_0 = NDR_PULL_GET_MEM_CTX(ndr);
			NDR_PULL_SET_MEM_CTX(ndr, r->sd, 0);
			{
				struct ndr_pull *_ndr_sd;
				NDR_CHECK(ndr_pull_subcontext_start(ndr, &_ndr_sd, 4, -1));
				NDR_CHECK(ndr_pull_security_descriptor(_ndr_sd, NDR_SCALARS|NDR_BUFFERS, r->sd));
				NDR_CHECK(ndr_pull_subcontext_end(ndr, _ndr_sd, 4, -1));
			}
			NDR_PULL_SET_MEM_CTX(ndr, _mem_save_sd_0, 0);
		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_sec_desc_buf(struct ndr_print *ndr, const char *name, const struct sec_desc_buf *r)
{
	ndr_print_struct(ndr, name, "sec_desc_buf");
	ndr->depth++;
	ndr_print_uint32(ndr, "sd_size", (ndr->flags & LIBNDR_PRINT_SET_VALUES)?ndr_size_security_descriptor(r->sd, ndr->iconv_convenience, ndr->flags):r->sd_size);
	ndr_print_ptr(ndr, "sd", r->sd);
	ndr->depth++;
	if (r->sd) {
		ndr_print_security_descriptor(ndr, "sd", r->sd);
	}
	ndr->depth--;
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_security_token(struct ndr_push *ndr, int ndr_flags, const struct security_token *r)
{
	uint32_t cntr_sids_0;
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_push_align(ndr, 5));
		NDR_CHECK(ndr_push_unique_ptr(ndr, r->user_sid));
		NDR_CHECK(ndr_push_unique_ptr(ndr, r->group_sid));
		NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r->num_sids));
		NDR_CHECK(ndr_push_uint3264(ndr, NDR_SCALARS, r->num_sids));
		for (cntr_sids_0 = 0; cntr_sids_0 < r->num_sids; cntr_sids_0++) {
			NDR_CHECK(ndr_push_unique_ptr(ndr, r->sids[cntr_sids_0]));
		}
		NDR_CHECK(ndr_push_udlong(ndr, NDR_SCALARS, r->privilege_mask));
		NDR_CHECK(ndr_push_unique_ptr(ndr, r->default_dacl));
		NDR_CHECK(ndr_push_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		if (r->user_sid) {
			NDR_CHECK(ndr_push_dom_sid(ndr, NDR_SCALARS, r->user_sid));
		}
		if (r->group_sid) {
			NDR_CHECK(ndr_push_dom_sid(ndr, NDR_SCALARS, r->group_sid));
		}
		for (cntr_sids_0 = 0; cntr_sids_0 < r->num_sids; cntr_sids_0++) {
			if (r->sids[cntr_sids_0]) {
				NDR_CHECK(ndr_push_dom_sid(ndr, NDR_SCALARS, r->sids[cntr_sids_0]));
			}
		}
		if (r->default_dacl) {
			NDR_CHECK(ndr_push_security_acl(ndr, NDR_SCALARS|NDR_BUFFERS, r->default_dacl));
		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_token(struct ndr_pull *ndr, int ndr_flags, struct security_token *r)
{
	uint32_t _ptr_user_sid;
	TALLOC_CTX *_mem_save_user_sid_0;
	uint32_t _ptr_group_sid;
	TALLOC_CTX *_mem_save_group_sid_0;
	uint32_t _ptr_sids;
	uint32_t cntr_sids_0;
	TALLOC_CTX *_mem_save_sids_0;
	TALLOC_CTX *_mem_save_sids_1;
	uint32_t _ptr_default_dacl;
	TALLOC_CTX *_mem_save_default_dacl_0;
	if (ndr_flags & NDR_SCALARS) {
		NDR_CHECK(ndr_pull_align(ndr, 5));
		NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_user_sid));
		if (_ptr_user_sid) {
			NDR_PULL_ALLOC(ndr, r->user_sid);
		} else {
			r->user_sid = NULL;
		}
		NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_group_sid));
		if (_ptr_group_sid) {
			NDR_PULL_ALLOC(ndr, r->group_sid);
		} else {
			r->group_sid = NULL;
		}
		NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &r->num_sids));
		NDR_CHECK(ndr_pull_array_size(ndr, &r->sids));
		NDR_PULL_ALLOC_N(ndr, r->sids, ndr_get_array_size(ndr, &r->sids));
		_mem_save_sids_0 = NDR_PULL_GET_MEM_CTX(ndr);
		NDR_PULL_SET_MEM_CTX(ndr, r->sids, 0);
		for (cntr_sids_0 = 0; cntr_sids_0 < r->num_sids; cntr_sids_0++) {
			NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_sids));
			if (_ptr_sids) {
				NDR_PULL_ALLOC(ndr, r->sids[cntr_sids_0]);
			} else {
				r->sids[cntr_sids_0] = NULL;
			}
		}
		NDR_PULL_SET_MEM_CTX(ndr, _mem_save_sids_0, 0);
		NDR_CHECK(ndr_pull_udlong(ndr, NDR_SCALARS, &r->privilege_mask));
		NDR_CHECK(ndr_pull_generic_ptr(ndr, &_ptr_default_dacl));
		if (_ptr_default_dacl) {
			NDR_PULL_ALLOC(ndr, r->default_dacl);
		} else {
			r->default_dacl = NULL;
		}
		if (r->sids) {
			NDR_CHECK(ndr_check_array_size(ndr, (void*)&r->sids, r->num_sids));
		}
		NDR_CHECK(ndr_pull_trailer_align(ndr, 5));
	}
	if (ndr_flags & NDR_BUFFERS) {
		if (r->user_sid) {
			_mem_save_user_sid_0 = NDR_PULL_GET_MEM_CTX(ndr);
			NDR_PULL_SET_MEM_CTX(ndr, r->user_sid, 0);
			NDR_CHECK(ndr_pull_dom_sid(ndr, NDR_SCALARS, r->user_sid));
			NDR_PULL_SET_MEM_CTX(ndr, _mem_save_user_sid_0, 0);
		}
		if (r->group_sid) {
			_mem_save_group_sid_0 = NDR_PULL_GET_MEM_CTX(ndr);
			NDR_PULL_SET_MEM_CTX(ndr, r->group_sid, 0);
			NDR_CHECK(ndr_pull_dom_sid(ndr, NDR_SCALARS, r->group_sid));
			NDR_PULL_SET_MEM_CTX(ndr, _mem_save_group_sid_0, 0);
		}
		_mem_save_sids_0 = NDR_PULL_GET_MEM_CTX(ndr);
		NDR_PULL_SET_MEM_CTX(ndr, r->sids, 0);
		for (cntr_sids_0 = 0; cntr_sids_0 < r->num_sids; cntr_sids_0++) {
			if (r->sids[cntr_sids_0]) {
				_mem_save_sids_1 = NDR_PULL_GET_MEM_CTX(ndr);
				NDR_PULL_SET_MEM_CTX(ndr, r->sids[cntr_sids_0], 0);
				NDR_CHECK(ndr_pull_dom_sid(ndr, NDR_SCALARS, r->sids[cntr_sids_0]));
				NDR_PULL_SET_MEM_CTX(ndr, _mem_save_sids_1, 0);
			}
		}
		NDR_PULL_SET_MEM_CTX(ndr, _mem_save_sids_0, 0);
		if (r->default_dacl) {
			_mem_save_default_dacl_0 = NDR_PULL_GET_MEM_CTX(ndr);
			NDR_PULL_SET_MEM_CTX(ndr, r->default_dacl, 0);
			NDR_CHECK(ndr_pull_security_acl(ndr, NDR_SCALARS|NDR_BUFFERS, r->default_dacl));
			NDR_PULL_SET_MEM_CTX(ndr, _mem_save_default_dacl_0, 0);
		}
	}
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_token(struct ndr_print *ndr, const char *name, const struct security_token *r)
{
	uint32_t cntr_sids_0;
	ndr_print_struct(ndr, name, "security_token");
	ndr->depth++;
	ndr_print_ptr(ndr, "user_sid", r->user_sid);
	ndr->depth++;
	if (r->user_sid) {
		ndr_print_dom_sid(ndr, "user_sid", r->user_sid);
	}
	ndr->depth--;
	ndr_print_ptr(ndr, "group_sid", r->group_sid);
	ndr->depth++;
	if (r->group_sid) {
		ndr_print_dom_sid(ndr, "group_sid", r->group_sid);
	}
	ndr->depth--;
	ndr_print_uint32(ndr, "num_sids", r->num_sids);
	ndr->print(ndr, "%s: ARRAY(%d)", "sids", (int)r->num_sids);
	ndr->depth++;
	for (cntr_sids_0=0;cntr_sids_0<r->num_sids;cntr_sids_0++) {
		char *idx_0=NULL;
		if (asprintf(&idx_0, "[%d]", cntr_sids_0) != -1) {
			ndr_print_ptr(ndr, "sids", r->sids[cntr_sids_0]);
			ndr->depth++;
			if (r->sids[cntr_sids_0]) {
				ndr_print_dom_sid(ndr, "sids", r->sids[cntr_sids_0]);
			}
			ndr->depth--;
			free(idx_0);
		}
	}
	ndr->depth--;
	ndr_print_udlong(ndr, "privilege_mask", r->privilege_mask);
	ndr_print_ptr(ndr, "default_dacl", r->default_dacl);
	ndr->depth++;
	if (r->default_dacl) {
		ndr_print_security_acl(ndr, "default_dacl", r->default_dacl);
	}
	ndr->depth--;
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_security_secinfo(struct ndr_push *ndr, int ndr_flags, uint32_t r)
{
	NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_secinfo(struct ndr_pull *ndr, int ndr_flags, uint32_t *r)
{
	uint32_t v;
	NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_secinfo(struct ndr_print *ndr, const char *name, uint32_t r)
{
	ndr_print_uint32(ndr, name, r);
	ndr->depth++;
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_OWNER", SECINFO_OWNER, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_GROUP", SECINFO_GROUP, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_DACL", SECINFO_DACL, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_SACL", SECINFO_SACL, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_UNPROTECTED_SACL", SECINFO_UNPROTECTED_SACL, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_UNPROTECTED_DACL", SECINFO_UNPROTECTED_DACL, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_PROTECTED_SACL", SECINFO_PROTECTED_SACL, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SECINFO_PROTECTED_DACL", SECINFO_PROTECTED_DACL, r);
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_kerb_EncTypes(struct ndr_push *ndr, int ndr_flags, uint32_t r)
{
	NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_kerb_EncTypes(struct ndr_pull *ndr, int ndr_flags, uint32_t *r)
{
	uint32_t v;
	NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_kerb_EncTypes(struct ndr_print *ndr, const char *name, uint32_t r)
{
	ndr_print_uint32(ndr, name, r);
	ndr->depth++;
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "KERB_ENCTYPE_DES_CBC_CRC", KERB_ENCTYPE_DES_CBC_CRC, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "KERB_ENCTYPE_DES_CBC_MD5", KERB_ENCTYPE_DES_CBC_MD5, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "KERB_ENCTYPE_RC4_HMAC_MD5", KERB_ENCTYPE_RC4_HMAC_MD5, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "KERB_ENCTYPE_AES128_CTS_HMAC_SHA1_96", KERB_ENCTYPE_AES128_CTS_HMAC_SHA1_96, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "KERB_ENCTYPE_AES256_CTS_HMAC_SHA1_96", KERB_ENCTYPE_AES256_CTS_HMAC_SHA1_96, r);
	ndr->depth--;
}

_PUBLIC_ enum ndr_err_code ndr_push_security_autoinherit(struct ndr_push *ndr, int ndr_flags, uint32_t r)
{
	NDR_CHECK(ndr_push_uint32(ndr, NDR_SCALARS, r));
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ enum ndr_err_code ndr_pull_security_autoinherit(struct ndr_pull *ndr, int ndr_flags, uint32_t *r)
{
	uint32_t v;
	NDR_CHECK(ndr_pull_uint32(ndr, NDR_SCALARS, &v));
	*r = v;
	return NDR_ERR_SUCCESS;
}

_PUBLIC_ void ndr_print_security_autoinherit(struct ndr_print *ndr, const char *name, uint32_t r)
{
	ndr_print_uint32(ndr, name, r);
	ndr->depth++;
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SEC_DACL_AUTO_INHERIT", SEC_DACL_AUTO_INHERIT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SEC_SACL_AUTO_INHERIT", SEC_SACL_AUTO_INHERIT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SEC_DEFAULT_DESCRIPTOR", SEC_DEFAULT_DESCRIPTOR, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SEC_OWNER_FROM_PARENT", SEC_OWNER_FROM_PARENT, r);
	ndr_print_bitmap_flag(ndr, sizeof(uint32_t), "SEC_GROUP_FROM_PARENT", SEC_GROUP_FROM_PARENT, r);
	ndr->depth--;
}

