#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__
#include <ZUI.h>
#include "mxml.h"
#include "tree.h"

typedef struct _ZTemplate
{
    RB_ENTRY(_ZTemplate) entry;
    mxml_node_t *node;
    unsigned int key;
}*ZuiTemplate, ZTemplate;
RB_HEAD(_ZTemplate_Tree, _ZTemplate);
RB_PROTOTYPE(_ZTemplate_Tree, _ZTemplate,,);

extern struct _ZTemplate_Tree *Global_TemplateClass;

ZuiBool ZuiTemplateInit();
ZuiVoid ZuiTemplateUnInit();
ZuiVoid ZuiAddTemplate(mxml_node_t *node);
ZuiVoid ZuiLoadTemplate(mxml_node_t *n, ZuiControl p, ZuiAny Param1, ZuiAny Param2);
#endif //__TEMPLATE_H__
