/*
 * inprogram.c - libfyaml inprogram YAML example
 *
 * Copyright (c) 2019 Pantelis Antoniou <pantelis.antoniou@konsulko.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <libfyaml.h>

struct dump_userdata {
	FILE *fp;
	bool colorize;
	bool visible;
};

static int do_output(struct fy_emitter *fye, enum fy_emitter_write_type type, const char *str, int len, void *userdata);


int main(int argc, char *argv[])
{
	static const char *yaml = 
		"invoice: 34843\n"
		"date   : !!str 2001-01-23\n"
		"bill-to: &id001\n"
		"    given:\n"
		"        - 'Chris'\n"
		"        - 'Zorro'\n"
		"    family : Dumars\n"
		"    address:\n"
		"        lines: |\n"
		"            458 Walkman Dr.\n"
		"            Suite #292\n";
	struct fy_document *fyd = NULL;
	struct fy_parser *fyp = NULL;
	struct fy_emitter *fye = NULL;
	struct fy_node *fyn = NULL;
	int rc, count, ret = EXIT_FAILURE;
	struct fy_emitter_cfg emit_cfg;
	struct dump_userdata du;

	struct fy_parse_cfg cfg = {
		.search_path = "",
		.flags = 0
	};

	fy_set_default_parser_cfg_flags(cfg.flags);
	fyp = fy_parser_create(&cfg);

	memset(&du, 0, sizeof(du));
	du.fp = stdout;
	du.colorize = true;
	du.visible = true;

	memset(&emit_cfg, 0, sizeof(emit_cfg));
	emit_cfg.flags = 0 | FYECF_INDENT(3) | FYECF_WIDTH(80);
	emit_cfg.output = do_output;
	emit_cfg.userdata = &du;
	fye = fy_emitter_create(&emit_cfg);

	fy_parser_set_string(fyp, yaml, strlen(yaml));
	fyd = fy_parse_load_document(fyp);
	
	fyn = fy_node_by_path(fy_document_root(fyd), "/bill-to/given/1", FY_NT, FYNWF_FOLLOW);

	rc = fy_emit_document_start(fye, fyd, fyn);
	rc = fy_emit_root_node(fye, fyn);
	rc = fy_emit_document_end(fye);

	fy_parse_document_destroy(fyp, fyd);

	return ret;
}

static inline int
utf8_width_by_first_octet(uint8_t c)
{
	return (c & 0x80) == 0x00 ? 1 :
	       (c & 0xe0) == 0xc0 ? 2 :
	       (c & 0xf0) == 0xe0 ? 3 :
	       (c & 0xf8) == 0xf0 ? 4 : 0;
}

static int do_output(struct fy_emitter *fye, enum fy_emitter_write_type type, const char *str, int len, void *userdata)
{
	struct dump_userdata *du = userdata;
	FILE *fp = du->fp;
	int ret, w;
	const char *color = NULL;
	const char *s, *e;

	s = str;
	e = str + len;
	if (du->colorize) {
		switch (type) {
		case fyewt_document_indicator:
			color = "\x1b[36m";
			break;
		case fyewt_tag_directive:
		case fyewt_version_directive:
			color = "\x1b[33m";
			break;
		case fyewt_indent:
			if (du->visible) {
				fputs("\x1b[32m", fp);
				while (s < e && (w = utf8_width_by_first_octet(((uint8_t)*s))) > 0) {
					/* open box - U+2423 */
					fputs("\xe2\x90\xa3", fp);
					s += w;
				}
				fputs("\x1b[0m", fp);
				return len;
			}
			break;
		case fyewt_indicator:
			if (len == 1 && (str[0] == '\'' || str[0] == '"'))
				color = "\x1b[33m";
			else if (len == 1 && str[0] == '&')
				color = "\x1b[32;1m";
			else
				color = "\x1b[35m";
			break;
		case fyewt_whitespace:
			if (du->visible) {
				fputs("\x1b[32m", fp);
				while (s < e && (w = utf8_width_by_first_octet(((uint8_t)*s))) > 0) {
					/* symbol for space - U+2420 */
					/* symbol for interpunct - U+00B7 */
					fputs("\xc2\xb7", fp);
					s += w;
				}
				fputs("\x1b[0m", fp);
				return len;
			}
			break;
		case fyewt_plain_scalar:
			color = "\x1b[37;1m";
			break;
		case fyewt_single_quoted_scalar:
		case fyewt_double_quoted_scalar:
			color = "\x1b[33m";
			break;
		case fyewt_literal_scalar:
		case fyewt_folded_scalar:
			color = "\x1b[33m";
			break;
		case fyewt_anchor:
		case fyewt_tag:
		case fyewt_alias:
			color = "\x1b[32;1m";
			break;
		case fyewt_linebreak:
			if (du->visible) {
				fputs("\x1b[32m", fp);
				while (s < e && (w = utf8_width_by_first_octet(((uint8_t)*s))) > 0) {
					/* symbol for space - ^M */
					/* fprintf(fp, "^M\n"); */
					/* down arrow - U+2193 */
					fputs("\xe2\x86\x93\n", fp);
					s += w;
				}
				fputs("\x1b[0m", fp);
				return len;
			}
			color = NULL;
			break;
		case fyewt_terminating_zero:
			color = NULL;
			break;
		case fyewt_plain_scalar_key:
		case fyewt_single_quoted_scalar_key:
		case fyewt_double_quoted_scalar_key:
			color = "\x1b[36;1m";
			break;
		case fyewt_comment:
			color = "\x1b[34;1m";
			break;
		}
	}

	/* don't output the terminating zero */
	if (type == fyewt_terminating_zero)
		return len;

	if (color)
		fputs(color, fp);

	ret = fwrite(str, 1, len, fp);

	if (color)
		fputs("\x1b[0m", fp);

	return ret;
}
