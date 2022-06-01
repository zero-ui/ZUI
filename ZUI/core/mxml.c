#include "mxml.h"
#include "function.h"
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#define strdup _wcsdup
#define mxml_bad_char(ch) ((ch) < L' ' && (ch) != L'\n' && (ch) != L'\r' && (ch) != L'\t')

/*判断字符是否为空白字符*/
static int mxml_isspace(int ch)
{
    return (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n');
}

//--------ATTR操作
static int mxml_set_attr(mxml_node_t *node, const wchar_t *name, wchar_t *value)
{
    int i; /* Looping var */
    mxml_attr_t *attr; /* New attribute */

    for (i = node->value.num_attrs, attr = node->value.attrs;
        i > 0;
        i--, attr++)
        if (!_tcsicmp(attr->name, name))
        {
            /*
            * Free the old value as needed...
            */

            if (attr->value) {
                free(attr->value);
                attr->value = NULL;
            }
            attr->value = value;

            return (0);
        }

    if (node->value.num_attrs == 0)
        attr = (mxml_attr_t *)memset(malloc(sizeof(mxml_attr_t)), 0, sizeof(mxml_attr_t));
    else
        attr = (mxml_attr_t *)realloc(node->value.attrs,
        (node->value.num_attrs + 1) * sizeof(mxml_attr_t));

    if (!attr)
    {
        return (-1);
    }

    node->value.attrs = attr;
    attr += node->value.num_attrs;

    if ((attr->name = strdup(name)) == NULL)
    {
        return (-1);
    }

    attr->value = value;

    node->value.num_attrs++;

    return (0);
}

wchar_t* mxmlElementGetAttr(mxml_node_t *node, const wchar_t *name)
{
    int i; /* Looping var */
    mxml_attr_t *attr; /* Cirrent attribute */


    if (!node || !name)
        return (NULL);

    for (i = node->value.num_attrs, attr = node->value.attrs;
        i > 0;
        i--, attr++)
    {

        if (!_tcsicmp(attr->name, name))
        {
            return (attr->value);
        }
    }
    return (NULL);
}

void mxmlElementSetAttr(mxml_node_t *node, const wchar_t *name, const wchar_t *value)
{
    wchar_t *valuec; /* Copy of value */

    if (!node || !name)
        return;

    if (value)
        valuec = strdup(value);
    else
        valuec = NULL;

    if (mxml_set_attr(node, name, valuec))
        free(valuec);
}
//-------节点操作
void mxmlRemove(mxml_node_t *node)
{
    if (!node || !node->parent)
        return;

    /*
    * Remove from parent...
    */
    if (node->prev)
        node->prev->next = node->next;
    else
        node->parent->child = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        node->parent->last_child = node->prev;

    node->parent = NULL;
    node->prev = NULL;
    node->next = NULL;
}

void mxmlAdd(mxml_node_t *parent, int where, mxml_node_t *child, mxml_node_t *node)
{
    if (!parent || !node)
        return;

    /*
    * Remove the node from any existing parent...
    */

    if (node->parent)
        mxmlRemove(node);

    /*
    * Reset pointers...
    */

    node->parent = parent;

    switch (where)
    {
    case MXML_ADD_BEFORE:
        if (!child || child == parent->child || child->parent != parent)
        {
            /*
            * Insert as first node under parent...
            */

            node->next = parent->child;

            if (parent->child)
                parent->child->prev = node;
            else
                parent->last_child = node;

            parent->child = node;
        }
        else
        {
            /*
            * Insert node before this child...
            */

            node->next = child;
            node->prev = child->prev;

            if (child->prev)
                child->prev->next = node;
            else
                parent->child = node;

            child->prev = node;
        }
        break;

    case MXML_ADD_AFTER:
        if (!child || child == parent->last_child || child->parent != parent)
        {
            /*
            * Insert as last node under parent...
            */

            node->parent = parent;
            node->prev = parent->last_child;

            if (parent->last_child)
                parent->last_child->next = node;
            else
                parent->child = node;

            parent->last_child = node;
        }
        else
        {
            /*
            * Insert node after this child...
            */

            node->prev = child;
            node->next = child->next;

            if (child->next)
                child->next->prev = node;
            else
                parent->last_child = node;

            child->next = node;
        }
        break;
    }
}

static mxml_node_t *mxml_new(mxml_node_t *parent)
{
    mxml_node_t *node; /* New node */

    if ((node = (mxml_node_t *)memset(malloc(sizeof(mxml_node_t)), 0, sizeof(mxml_node_t))) == NULL)
    {
        return (NULL);
    }

    if (parent)
        mxmlAdd(parent, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, node);
    //返回节点
    return (node);
}

void mxmlDelete(mxml_node_t *node)
{
    int i; /* Looping var */

    if (!node)
        return;

    mxmlRemove(node);

    if (node->value.num_attrs)
    {
        for (i = 0; i < node->value.num_attrs; i++)
        {
            if (node->value.attrs[i].name) {
                free(node->value.attrs[i].name);
            }
            if (node->value.attrs[i].value) {
                free(node->value.attrs[i].value);
            }
        }

        free(node->value.attrs);
        node->value.attrs = NULL;
    }

    if (node->value.name) {
        free(node->value.name);
        node->value.name = NULL;
    }

    //删除子节点
    while (node->child)
        mxmlDelete(node->child);

    //释放节点内存
    free(node);
}
mxml_node_t *mxmlClone(mxml_node_t *node, mxml_node_t *parent) {
    mxml_node_t *new_node = mxml_new(parent);
    new_node->parent = parent;
    new_node->user_data = node->user_data;
    new_node->value.name = strdup(node->value.name);
    new_node->value.num_attrs = node->value.num_attrs;
    if (node->value.num_attrs)
        new_node->value.attrs = (mxml_attr_t *)memset(malloc(node->value.num_attrs * sizeof(mxml_attr_t)), 0, node->value.num_attrs * sizeof(mxml_attr_t));
    for (int i = 0; i < node->value.num_attrs; i++)
    {
        new_node->value.attrs[i].name = strdup(node->value.attrs[i].name);
        new_node->value.attrs[i].value = strdup(node->value.attrs[i].value);
    }
    if (node->child)
        mxmlClone(node->child, new_node);
    if (parent)
        if (node->next)
            mxmlClone(node->next, parent);
    return new_node;
}
mxml_node_t *mxmlNewElement(mxml_node_t *parent, const wchar_t *name)
{
    mxml_node_t *node; /* New node */

    if (!name)
        return (NULL);

    /*
    * Create the node and set the element name...
    */

    if ((node = mxml_new(parent)) != NULL)
        node->value.name = strdup(name);

    return (node);
}

int mxmlRelease(mxml_node_t *node)
{
    if (node)
    {
        mxmlDelete(node);
        return (0);
    }
    else
        return (-1);
}
//--------XML树解析
typedef wchar_t(*mxml_getc)(mxml_buf_t *p);
//取字符
static wchar_t mxml_string_getc(mxml_buf_t *p)
{
    wchar_t ch = 0; /* Character */
    const wchar_t **s; /* Pointer to string pointer */
    s = (const wchar_t **)p;
    if (p->pos < p->len)
    {
        ch = p->buf[p->pos];


        p->pos++;
        if (ch != 0)
        {
            if (ch == 0xfeff)
                return (mxml_string_getc(p));
            if (ch == 65535)
                return (mxml_string_getc(p));
            if (mxml_bad_char(ch))
            {
                return (WEOF);
            }
            return (ch);
        }
    }
    return WEOF;
}

static int mxml_add_char(wchar_t ch, wchar_t **bufptr, wchar_t **buffer, int *bufsize)
{
    wchar_t *newbuffer; /* New buffer value */


    if (*bufptr >= (*buffer + *bufsize))
    {
        /*
        * Increase the size of the buffer...
        */

        if (*bufsize < 1024)
            (*bufsize) *= 2;
        else
            (*bufsize) += 1024;

        if ((newbuffer = (wchar_t *)realloc(*buffer, (*bufsize) * sizeof(wchar_t))) == NULL)
        {
            free(*buffer);
            *buffer = NULL;
            return (-1);
        }

        *bufptr = newbuffer + (*bufptr - *buffer);
        *buffer = newbuffer;
    }

    *(*bufptr)++ = ch;
    return (0);
}
//查找转义符
static int mxmlEntityGetValue(const wchar_t *name)
{
    int diff, /* Difference between names */
        current, /* Current entity in search */
        first, /* First entity in search */
        last; /* Last entity in search */

    static const struct
    {
        const wchar_t *name; /* Entity name */
        int val; /* Character value */
    } entities[] =
    {
    { L"AElig", 198 },
    { L"Aacute", 193 },
    { L"Acirc", 194 },
    { L"Agrave", 192 },
    { L"Alpha", 913 },
    { L"Aring", 197 },
    { L"Atilde", 195 },
    { L"Auml", 196 },
    { L"Beta", 914 },
    { L"Ccedil", 199 },
    { L"Chi", 935 },
    { L"Dagger", 8225 },
    { L"Delta", 916 },
    { L"Dstrok", 208 },
    { L"ETH", 208 },
    { L"Eacute", 201 },
    { L"Ecirc", 202 },
    { L"Egrave", 200 },
    { L"Epsilon", 917 },
    { L"Eta", 919 },
    { L"Euml", 203 },
    { L"Gamma", 915 },
    { L"Iacute", 205 },
    { L"Icirc", 206 },
    { L"Igrave", 204 },
    { L"Iota", 921 },
    { L"Iuml", 207 },
    { L"Kappa", 922 },
    { L"Lambda", 923 },
    { L"Mu", 924 },
    { L"Ntilde", 209 },
    { L"Nu", 925 },
    { L"OElig", 338 },
    { L"Oacute", 211 },
    { L"Ocirc", 212 },
    { L"Ograve", 210 },
    { L"Omega", 937 },
    { L"Omicron", 927 },
    { L"Oslash", 216 },
    { L"Otilde", 213 },
    { L"Ouml", 214 },
    { L"Phi", 934 },
    { L"Pi", 928 },
    { L"Prime", 8243 },
    { L"Psi", 936 },
    { L"Rho", 929 },
    { L"Scaron", 352 },
    { L"Sigma", 931 },
    { L"THORN", 222 },
    { L"Tau", 932 },
    { L"Theta", 920 },
    { L"Uacute", 218 },
    { L"Ucirc", 219 },
    { L"Ugrave", 217 },
    { L"Upsilon", 933 },
    { L"Uuml", 220 },
    { L"Xi", 926 },
    { L"Yacute", 221 },
    { L"Yuml", 376 },
    { L"Zeta", 918 },
    { L"aacute", 225 },
    { L"acirc", 226 },
    { L"acute", 180 },
    { L"aelig", 230 },
    { L"agrave", 224 },
    { L"alefsym", 8501 },
    { L"alpha", 945 },
    { L"amp", '&' },
    { L"and", 8743 },
    { L"ang", 8736 },
    { L"apos", '\'' },
    { L"aring", 229 },
    { L"asymp", 8776 },
    { L"atilde", 227 },
    { L"auml", 228 },
    { L"bdquo", 8222 },
    { L"beta", 946 },
    { L"brkbar", 166 },
    { L"brvbar", 166 },
    { L"bull", 8226 },
    { L"cap", 8745 },
    { L"ccedil", 231 },
    { L"cedil", 184 },
    { L"cent", 162 },
    { L"chi", 967 },
    { L"circ", 710 },
    { L"clubs", 9827 },
    { L"cong", 8773 },
    { L"copy", 169 },
    { L"crarr", 8629 },
    { L"cup", 8746 },
    { L"curren", 164 },
    { L"dArr", 8659 },
    { L"dagger", 8224 },
    { L"darr", 8595 },
    { L"deg", 176 },
    { L"delta", 948 },
    { L"diams", 9830 },
    { L"die", 168 },
    { L"divide", 247 },
    { L"eacute", 233 },
    { L"ecirc", 234 },
    { L"egrave", 232 },
    { L"empty", 8709 },
    { L"emsp", 8195 },
    { L"ensp", 8194 },
    { L"epsilon", 949 },
    { L"equiv", 8801 },
    { L"eta", 951 },
    { L"eth", 240 },
    { L"euml", 235 },
    { L"euro", 8364 },
    { L"exist", 8707 },
    { L"fnof", 402 },
    { L"forall", 8704 },
    { L"frac12", 189 },
    { L"frac14", 188 },
    { L"frac34", 190 },
    { L"frasl", 8260 },
    { L"gamma", 947 },
    { L"ge", 8805 },
    { L"gt", '>' },
    { L"hArr", 8660 },
    { L"harr", 8596 },
    { L"hearts", 9829 },
    { L"hellip", 8230 },
    { L"hibar", 175 },
    { L"iacute", 237 },
    { L"icirc", 238 },
    { L"iexcl", 161 },
    { L"igrave", 236 },
    { L"image", 8465 },
    { L"infin", 8734 },
    { L"int", 8747 },
    { L"iota", 953 },
    { L"iquest", 191 },
    { L"isin", 8712 },
    { L"iuml", 239 },
    { L"kappa", 954 },
    { L"lArr", 8656 },
    { L"lambda", 955 },
    { L"lang", 9001 },
    { L"laquo", 171 },
    { L"larr", 8592 },
    { L"lceil", 8968 },
    { L"ldquo", 8220 },
    { L"le", 8804 },
    { L"lfloor", 8970 },
    { L"lowast", 8727 },
    { L"loz", 9674 },
    { L"lrm", 8206 },
    { L"lsaquo", 8249 },
    { L"lsquo", 8216 },
    { L"lt", '<' },
    { L"macr", 175 },
    { L"mdash", 8212 },
    { L"micro", 181 },
    { L"middot", 183 },
    { L"minus", 8722 },
    { L"mu", 956 },
    { L"nabla", 8711 },
    { L"nbsp", 160 },
    { L"ndash", 8211 },
    { L"ne", 8800 },
    { L"ni", 8715 },
    { L"not", 172 },
    { L"notin", 8713 },
    { L"nsub", 8836 },
    { L"ntilde", 241 },
    { L"nu", 957 },
    { L"oacute", 243 },
    { L"ocirc", 244 },
    { L"oelig", 339 },
    { L"ograve", 242 },
    { L"oline", 8254 },
    { L"omega", 969 },
    { L"omicron", 959 },
    { L"oplus", 8853 },
    { L"or", 8744 },
    { L"ordf", 170 },
    { L"ordm", 186 },
    { L"oslash", 248 },
    { L"otilde", 245 },
    { L"otimes", 8855 },
    { L"ouml", 246 },
    { L"para", 182 },
    { L"part", 8706 },
    { L"permil", 8240 },
    { L"perp", 8869 },
    { L"phi", 966 },
    { L"pi", 960 },
    { L"piv", 982 },
    { L"plusmn", 177 },
    { L"pound", 163 },
    { L"prime", 8242 },
    { L"prod", 8719 },
    { L"prop", 8733 },
    { L"psi", 968 },
    { L"quot", '\"' },
    { L"rArr", 8658 },
    { L"radic", 8730 },
    { L"rang", 9002 },
    { L"raquo", 187 },
    { L"rarr", 8594 },
    { L"rceil", 8969 },
    { L"rdquo", 8221 },
    { L"real", 8476 },
    { L"reg", 174 },
    { L"rfloor", 8971 },
    { L"rho", 961 },
    { L"rlm", 8207 },
    { L"rsaquo", 8250 },
    { L"rsquo", 8217 },
    { L"sbquo", 8218 },
    { L"scaron", 353 },
    { L"sdot", 8901 },
    { L"sect", 167 },
    { L"shy", 173 },
    { L"sigma", 963 },
    { L"sigmaf", 962 },
    { L"sim", 8764 },
    { L"spades", 9824 },
    { L"sub", 8834 },
    { L"sube", 8838 },
    { L"sum", 8721 },
    { L"sup", 8835 },
    { L"sup1", 185 },
    { L"sup2", 178 },
    { L"sup3", 179 },
    { L"supe", 8839 },
    { L"szlig", 223 },
    { L"tau", 964 },
    { L"there4", 8756 },
    { L"theta", 952 },
    { L"thetasym", 977 },
    { L"thinsp", 8201 },
    { L"thorn", 254 },
    { L"tilde", 732 },
    { L"times", 215 },
    { L"trade", 8482 },
    { L"uArr", 8657 },
    { L"uacute", 250 },
    { L"uarr", 8593 },
    { L"ucirc", 251 },
    { L"ugrave", 249 },
    { L"uml", 168 },
    { L"upsih", 978 },
    { L"upsilon", 965 },
    { L"uuml", 252 },
    { L"weierp", 8472 },
    { L"xi", 958 },
    { L"yacute", 253 },
    { L"yen", 165 },
    { L"yuml", 255 },
    { L"zeta", 950 },
    { L"zwj", 8205 },
    { L"zwnj", 8204 }
    };


    /*
    * Do a binary search for the named entity...
    */

    first = 0;
    last = (int)(sizeof(entities) / sizeof(entities[0]) - 1);

    while ((last - first) > 1)
    {
        current = (first + last) / 2;

        if ((diff = _tcsicmp(name, entities[current].name)) == 0)
            return (entities[current].val);
        else if (diff < 0)
            last = current;
        else
            first = current;
    }

    /*
    * If we get here, there is a small chance that there is still
    * a match; check first and last...
    */

    if (!_tcsicmp(name, entities[first].name))
        return (entities[first].val);
    else if (!_tcsicmp(name, entities[last].name))
        return (entities[last].val);
    else
        return (-1);
}
//处理转义符号
static int mxml_get_entity(mxml_node_t *parent, void *p, mxml_getc mxml_string_getc)
{
    int ch; /* Current character */
    wchar_t entity[64], /* Entity string */
        *entptr; /* Pointer into entity */

    entptr = entity;

    while ((ch = mxml_string_getc(p)) != WEOF)
        if (ch > 126 || (!iswalnum(ch) && ch != L'#'))
            break;
        else if (entptr < (entity + sizeof(entity) - 1))
            *entptr++ = ch;
        else
        {
            break;
        }

    *entptr = L'\0';

    if (ch != L';')
    {
        return (WEOF);
    }

    if (entity[0] == L'#')
    {
        if (entity[1] == L'x')
            ch = wcstol(entity + 2, NULL, 16);
        else
            ch = wcstol(entity + 1, NULL, 10);
    }
    else
        ch = mxmlEntityGetValue(entity);

    if (mxml_bad_char(ch))
    {
        return (WEOF);
    }

    return (ch);
}

static int mxml_parse_element(mxml_node_t *node, void *p, mxml_getc mxml_string_getc)
{
    wchar_t ch, /* Current character in file */
        quote; /* Quoting character */
    wchar_t *name, /* Attribute name */
        *value, /* Attribute value */
        *ptr; /* Pointer into name/value */
    int namesize, /* Size of name string */
        valsize; /* Size of value string */


        /*
        * Initialize the name and value buffers...
        */

    if ((name = (wchar_t *)memset(malloc(64 * sizeof(wchar_t)), 0, 64 * sizeof(wchar_t))) == NULL)
    {
        return (WEOF);
    }

    namesize = 64;

    if ((value = (wchar_t *)memset(malloc(64 * sizeof(wchar_t)), 0, 64 * sizeof(wchar_t))) == NULL)
    {
        free(name);
        return (WEOF);
    }

    valsize = 64;

    /*
    * Loop until we hit a >, /, ?, or EOF...
    */

    while ((ch = mxml_string_getc(p)) != WEOF)
    {
        if (mxml_isspace(ch))
            continue;

        /*
        * Stop at /, ?, or >...
        */

        if (ch == L'/' || ch == L'?')
        {
            /*
            * Grab the > character and print an error if it isn't there...
            */

            quote = mxml_string_getc(p);

            if (quote != L'>')
            {
                goto error;
            }

            break;
        }
        else if (ch == L'<')
        {
            goto error;
        }
        else if (ch == L'>')
            break;

        /*
        * Read the attribute name...
        */


        if (ch == L'\"' || ch == L'\'')
        {
            /*
            * Name is in quotes, so get a quoted string...
            */

            quote = ch;
            ptr = name;

            while ((ch = mxml_string_getc(p)) != WEOF)
            {
                if (ch == quote) {
                    ch = mxml_string_getc(p);
                    break;
                }
                if (ch == L'&') {
                    if ((ch = mxml_get_entity(node, p, mxml_string_getc)) == WEOF)
                        goto error;
                    if (ch == L';')
                        continue;//跳过分号
                }
                if (mxml_add_char(ch, &ptr, &name, &namesize))
                    goto error;

            }
        }
        else
        {
            /*
            * Grab an normal, non-quoted name...
            */

            name[0] = ch;
            ptr = name + 1;
            while ((ch = mxml_string_getc(p)) != WEOF)
                if (mxml_isspace(ch) || ch == L'=' || ch == L'/' || ch == L'>' ||
                    ch == L'?')
                    break;
                else
                {
                    if (ch == L'&') {
                        if ((ch = mxml_get_entity(node, p, mxml_string_getc)) == WEOF)
                            goto error;
                        if (ch == ';')
                            continue;//跳过分号
                    }
                    if (mxml_add_char(ch, &ptr, &name, &namesize))
                        goto error;
                }
        }

        *ptr = L'\0';

        if (mxmlElementGetAttr(node, name))
            goto error;

        while (ch != WEOF && mxml_isspace(ch))
            ch = mxml_string_getc(p);

        if (ch == L'=')
        {
            /*
            * Read the attribute value...
            */

            while ((ch = mxml_string_getc(p)) != WEOF && mxml_isspace(ch));

            if (ch == WEOF)
            {
                goto error;
            }

            if (ch == L'\'' || ch == L'\"')
            {
                /*
                * Read quoted value...
                */

                quote = ch;
                ptr = value;

                while ((ch = mxml_string_getc(p)) != WEOF)
                    if (ch == quote)
                        break;
                    else
                    {
                        if (ch == L'&') {
                            if ((ch = mxml_get_entity(node, p, mxml_string_getc)) == WEOF)
                                goto error;
                            if (ch == L';')
                                continue;//跳过分号
                        }
                        if (mxml_add_char(ch, &ptr, &value, &valsize))
                            goto error;
                    }

                *ptr = L'\0';
            }
            else
            {
                /*
                * Read unquoted value...
                */

                value[0] = ch;
                ptr = value + 1;

                while ((ch = mxml_string_getc(p)) != WEOF)
                    if (mxml_isspace(ch) || ch == L'=' || ch == L'/' || ch == L'>')
                        break;
                    else
                    {
                        if (ch == L'&') {
                            if ((ch = mxml_get_entity(node, p, mxml_string_getc)) == WEOF)
                                goto error;
                            if (ch == L';')
                                continue;//跳过分号
                        }
                        if (mxml_add_char(ch, &ptr, &value, &valsize))
                            goto error;
                    }

                *ptr = L'\0';
            }

            /*
            * Set the attribute with the given string value...
            */

            mxmlElementSetAttr(node, name, value);
        }
        else
        {
            goto error;
        }

        /*
        * Check the end character...
        */

        if (ch == L'/' || ch == L'?')
        {
            /*
            * Grab the > character and print an error if it isn't there...
            */

            quote = mxml_string_getc(p);

            if (quote != L'>')
            {
                ch = WEOF;
            }

            break;
        }
        else if (ch == L'>')
            break;
    }

    /*
    * Free the name and value buffers and return...
    */

    free(name);
    free(value);

    return (ch);

error:

    free(name);
    free(value);

    return (WEOF);
}
//加载XML字符串 返回XML树
mxml_node_t *mxmlLoadString(mxml_node_t *top, ZuiAny s, int len)
{
    mxml_node_t *node, /* Current node */
        *first, /* First node added */
        *parent; /* Current parent node */
    wchar_t ch; /* Non-zero if whitespace seen */
    wchar_t *buffer, /* String buffer */
        *bufptr; /* Pointer into buffer */
    size_t bufsize = 0; /* Size of buffer */
    wchar_t *txtbuf = 0;
    if (ZuiStingIsUtf8(s, len))
    {
        bufsize = ZuiUtf8ToUnicode(s, len, 0, 0) * sizeof(wchar_t);
        txtbuf = malloc(bufsize + 2);
        bufsize = ZuiUtf8ToUnicode(s, len, txtbuf, bufsize);
        txtbuf[bufsize] = 0;
    }
    else
    {
        bufsize = ZuiAsciiToUnicode(s, len, 0, 0) * sizeof(wchar_t);
        txtbuf = malloc(bufsize + 2);
        bufsize = ZuiAsciiToUnicode(s, len, txtbuf, bufsize);
        txtbuf[bufsize] = 0;
    }
    mxml_buf_t buf;
    buf.buf = txtbuf;
    buf.len = bufsize;
    buf.pos = 0;
    mxml_buf_t *p = &buf;
    if ((buffer = (wchar_t *)malloc(64 * sizeof(wchar_t))) == NULL)
        return (NULL);
    bufsize = 64 * sizeof(wchar_t);
    bufptr = buffer;
    parent = top;
    first = NULL;

    while ((ch = mxml_string_getc(p)) != WEOF)
    {
        if ((ch == L'<' || mxml_isspace(ch)) && bufptr > buffer)
        {
            *bufptr = L'\0';

            node = mxmlNewElement(parent, buffer);

            if (*bufptr)
            {
                break;
            }

            bufptr = buffer;

            if (!first && node)
                first = node;
        }

        if (ch == L'<')
        {
            bufptr = buffer;

            while ((ch = mxml_string_getc(p)) != WEOF)
                if (mxml_isspace(ch) || ch == L'>' || (ch == L'/' && bufptr > buffer))
                    break;
                else if (ch == L'<')
                {
                    goto error;
                }
                else if (ch == L'&')
                {
                    if ((ch = mxml_get_entity(parent, p, mxml_string_getc)) == WEOF)
                        goto error;
                    if (ch == L';')
                        continue;//跳过分号
                    if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                        goto error;
                }
                else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                    goto error;
                else if (((bufptr - buffer) == 1 && buffer[0] == L'?') ||
                    ((bufptr - buffer) == 3 && !wcsncmp(buffer, L"!--", 3)) ||
                    ((bufptr - buffer) == 8 && !wcsncmp(buffer, L"![CDATA[", 8)))
                    break;

            *bufptr = L'\0';

            if (!_tcsicmp(buffer, L"!--"))
            {
                while ((ch = mxml_string_getc(p)) != WEOF)
                {
                    if (ch == L'>' && bufptr > (buffer + 4) &&
                        bufptr[-3] != L'-' && bufptr[-2] == L'-' && bufptr[-1] == L'-')
                        break;
                    else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                        goto error;
                }

                if (ch != L'>')
                {
                    goto error;
                }

                *bufptr = L'\0';

                if (!parent && first)
                {
                    goto error;
                }
                //不解析跳过注释
                node = NULL;
            }
            else if (!_tcsicmp(buffer, L"![CDATA["))
            {
                while ((ch = mxml_string_getc(p)) != WEOF)
                {
                    if (ch == L'>' && !wcsncmp(bufptr - 2, L"]]", 2))
                        break;
                    else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                        goto error;
                }

                if (ch != L'>')
                {
                    goto error;
                }

                *bufptr = '\0';

                if (!parent && first)
                {
                    goto error;
                }

                if ((node = mxmlNewElement(parent, buffer)) == NULL)
                {
                    goto error;
                }

                if (node && !first)
                    first = node;
            }
            else if (buffer[0] == L'?')
            {
                while ((ch = mxml_string_getc(p)) != WEOF)
                {
                    if (ch == L'>' && bufptr > buffer && bufptr[-1] == L'?')
                        break;
                    else if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                        goto error;
                }
                if (ch != L'>')
                {
                    goto error;
                }

                *bufptr = L'\0';

                if (!parent && first)
                {
                    goto error;
                }

                if ((node = mxmlNewElement(parent, buffer)) == NULL)
                {
                    goto error;
                }

                if (node)
                {
                    if (!first)
                        first = node;

                    if (!parent)
                    {
                        parent = node;
                    }
                }
            }
            else if (buffer[0] == L'!')
            {
                do
                {
                    if (ch == L'>')
                        break;
                    else
                    {
                        if (ch == L'&') {
                            if ((ch = mxml_get_entity(parent, p, mxml_string_getc)) == WEOF)
                                goto error;
                            if (ch == L';')
                                continue;//跳过分号
                        }
                        if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                            goto error;
                    }
                } while ((ch = mxml_string_getc(p)) != WEOF);

                if (ch != L'>')
                {
                    goto error;
                }

                *bufptr = L'\0';

                if (!parent && first)
                {
                    goto error;
                }

                if ((node = mxmlNewElement(parent, buffer)) == NULL)
                {
                    goto error;
                }

                if (node)
                {
                    if (!first)
                        first = node;

                    if (!parent)
                    {
                        parent = node;
                    }
                }
            }
            else if (buffer[0] == L'/')
            {
                if (!parent || _tcsicmp(buffer + 1, parent->value.name))
                {
                    goto error;
                }

                while (ch != L'>' && ch != WEOF)
                    ch = mxml_string_getc(p);

                node = parent;
                parent = parent->parent;
            }
            else
            {
                if (!parent && first)
                {
                    goto error;
                }

                if ((node = mxmlNewElement(parent, buffer)) == NULL)
                {
                    goto error;
                }

                if (mxml_isspace(ch))
                {
                    if ((ch = mxml_parse_element(node, p, mxml_string_getc)) == WEOF)
                        goto error;
                }
                else if (ch == L'/')
                {
                    if ((ch = mxml_string_getc(p)) != L'>')
                    {
                        mxmlDelete(node);
                        goto error;
                    }

                    ch = L'/';
                }

                if (!first)
                    first = node;

                if (ch == WEOF)
                    break;

                if (ch != L'/')
                {
                    parent = node;
                }
            }

            bufptr = buffer;
        }
        else if (ch == L'&')
        {
            if ((ch = mxml_get_entity(parent, p, mxml_string_getc)) == WEOF)
                goto error;
            if (ch == L';')
                continue;//跳过分号
            if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                goto error;
        }
        else if (!mxml_isspace(ch))
        {
            if (mxml_add_char(ch, &bufptr, &buffer, &bufsize))
                goto error;
        }
    }

    free(buffer);
    buffer = NULL;
    if (txtbuf) {
        free(txtbuf);
        txtbuf = NULL;
    }
    if (parent)
    {
        node = parent;

        while (parent != top && parent->parent)
            parent = parent->parent;

        if (node != parent)
        {
            mxmlDelete(first);
            return (NULL);
        }
    }
    if (parent)
        return (parent);
    else
        return (first);
error:

    mxmlDelete(first);
    free(buffer);
    return (NULL);
}
//-------节点查找
mxml_node_t *mxmlFindElement(mxml_node_t *node, mxml_node_t *top, const wchar_t *name, const wchar_t *attr, const wchar_t *value, int descend)
{
    wchar_t* temp; /* Current attribute value */


    /*
    * Range check input...
    */

    if (!node || !top || (!attr && value))
        return (NULL);

    /*
    * Start with the next node...
    */

    node = mxmlWalkNext(node, top, descend);

    /*
    * Loop until we find a matching element...
    */

    while (node != NULL)
    {
        /*
        * See if this node matches...
        */

        if (node->value.name &&
            (!name || !_tcsicmp(node->value.name, name)))
        {
            /*
            * See if we need to check for an attribute...
            */

            if (!attr)
                return (node); /* No attribute search, return it... */

                /*
                * Check for the attribute...
                */

            if ((temp = mxmlElementGetAttr(node, attr)) != NULL)
            {
                /*
                * OK, we have the attribute, does it match?
                */

                if (!value || !_tcsicmp(value, temp))
                    return (node); /* Yes, return it... */
            }
        }

        /*
        * No match, move on to the next node...
        */

        if (descend == MXML_DESCEND)
            node = mxmlWalkNext(node, top, MXML_DESCEND);
        else
            node = node->next;
    }

    return (NULL);
}

mxml_node_t *mxmlFindPath(mxml_node_t *top, const wchar_t *path)
{
    mxml_node_t *node; /* Current node */
    wchar_t element[256]; /* Current element name */
    const wchar_t *pathsep; /* Separator in path */
    int descend; /* mxmlFindElement option */


    /*
    * Range check input...
    */

    if (!top || !path || !*path)
        return (NULL);

    /*
    * Search each element in the path...
    */

    node = top;
    while (*path)
    {
        /*
        * Handle wildcards...
        */

        if (!wcsncmp(path, L"*/", 2))
        {
            path += 2;
            descend = MXML_DESCEND;
        }
        else
            descend = MXML_DESCEND_FIRST;

        /*
        * Get the next element in the path...
        */

        if ((pathsep = wcschr(path, L'/')) == NULL)
            pathsep = path + wcslen(path);

        if (pathsep == path || (pathsep - path) >= sizeof(element))
            return (NULL);

        memcpy(element, path, (pathsep - path)*sizeof(wchar_t));
        element[pathsep - path] = '\0';

        if (*pathsep)
            path = pathsep + 1;
        else
            path = pathsep;

        /*
        * Search for the element...
        */

        if ((node = mxmlFindElement(node, node, element, NULL, NULL,
            descend)) == NULL)
            return (NULL);
    }

    /*
    * If we get this far, return the node or its first child...
    */

    if (node->child)
        return (node->child);
    else
        return (node);
}

mxml_node_t *mxmlWalkNext(mxml_node_t *node, mxml_node_t *top, int descend)
{
    if (!node)
        return (NULL);
    else if (node->child && descend)
        return (node->child);
    else if (node == top)
        return (NULL);
    else if (node->next)
        return (node->next);
    else if (node->parent && node->parent != top)
    {
        node = node->parent;

        while (!node->next)
            if (node->parent == top || !node->parent)
                return (NULL);
            else
                node = node->parent;

        return (node->next);
    }
    else
        return (NULL);
}

mxml_node_t *mxmlWalkPrev(mxml_node_t *node, mxml_node_t *top, int descend)
{
    if (!node || node == top)
        return (NULL);
    else if (node->prev)
    {
        if (node->prev->last_child && descend)
        {
            /*
            * Find the last child under the previous node...
            */

            node = node->prev->last_child;

            while (node->last_child)
                node = node->last_child;

            return (node);
        }
        else
            return (node->prev);
    }
    else if (node->parent != top)
        return (node->parent);
    else
        return (NULL);
}

