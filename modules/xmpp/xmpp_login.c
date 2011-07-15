#include "util.h"
#include "connect.h"
#include "eventloop.h"
#include "xmlnode.h"

#include "xmpp_login.h"
#include "xmpp_parser.h"

static gchar *create_initiate_stream(XmppStream *xs);

/**
 * Strip the '/' from a label,make it an unclosed xml label.
 */
static void
strip_end_label(gchar *xml_string)
{
	gint length;

	g_return_if_fail(xml_string != NULL);

	if ((length = strlen(xml_string)) < 2) {
		return;
	}

	if (xml_string[length - 1] == '>' && xml_string[length - 2] == '/') {
		xml_string[length - 2] = '>';
		xml_string[length - 1] = '\0';
	}
}

static gboolean
init_stream_cb(gint sk, XmppStream *stream)
{
	gchar buf[BUF_LENGTH];
	gint n;

	if ((n = recv(sk, buf, strlen(buf), 0)) == -1) {
		
		hybrid_debug_error("xmpp", "init stream error.");

		return FALSE;
	}

	buf[n] = '\0';

	if (n > 0) {
		printf("%s\n", buf);

		xmpp_process_pushed(stream, buf, n);
	}

	return TRUE;
}

gboolean
init_connect(gint sk, XmppStream *xs)
{
	gchar *msg;

	/* send version. */
	msg = "<?xml version='1.0' ?>";

	if (send(sk, msg, strlen(msg), 0) == -1) {

		hybrid_debug_error("xmpp", "send initial jabber request failed");

		return FALSE;
	}

	/* send initiate stream request. */
	msg = create_initiate_stream(xs);

	strip_end_label(msg);

	hybrid_debug_info("xmpp", "send initite jabber stream:\n%s", msg);

	if (send(sk, msg, strlen(msg), 0) == -1) {

		hybrid_debug_error("xmpp", "send initial jabber request failed");

		g_free(msg);

		return FALSE;
	}

	g_free(msg);

	hybrid_event_add(sk, HYBRID_EVENT_READ, (input_func)init_stream_cb, xs);

	return FALSE;
}

/**
 * Create the initiate stream string.
 */
static gchar*
create_initiate_stream(XmppStream *xs)
{
	xmlnode *node;
	gchar *version;
	gchar *res;

	node = xmlnode_create("stream:stream");

	xmlnode_new_namespace(node, NULL, "jabber:client");
	xmlnode_new_namespace(node, "stream", "http://etherx.jabber.org/streams");

	version = g_strdup_printf("%d.%d", xs->major_version, xs->miner_version);

	xmlnode_new_prop(node, "version", version);

	g_free(version);

	res = xmlnode_to_string(node);

	xmlnode_free(node);

	return res;
}