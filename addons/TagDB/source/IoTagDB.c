/*#io
TagDB ioDoc(
	docCopyright("Steve Dekorte", 2007)
	docLicense("BSD revised")
	docDescription("""Binding for tagdb - a tagging database.""")
*/

#include "IoTagDB.h"
#include "IoState.h"
#include "IoNumber.h"
#include "IoSeq.h"
#include "IoList.h"
#include "TagDB.h"

#define DATA(self) ((TagDB *)IoObject_dataPointer(self))

IoTag *IoTagDB_newTag(void *state)
{
	IoTag *tag = IoTag_newWithName_("TagDB");
	IoTag_state_(tag, state);
	IoTag_cloneFunc_(tag, (IoTagCloneFunc *)IoTagDB_rawClone);
	IoTag_freeFunc_(tag, (IoTagFreeFunc *)IoTagDB_free);
	return tag;
}


IoTagDB *IoTagDB_rawClone(IoTagDB *proto)
{
	IoObject *self = IoObject_rawClonePrimitive(proto);
	IoObject_setDataPointer_(self, TagDB_new());
	return self;
}

IoTagDB *IoTagDB_new(void *state)
{
	IoObject *proto = IoState_protoWithInitFunction_(state, IoTagDB_proto);
	return IOCLONE(proto);
}

void IoTagDB_free(IoTagDB *self)
{
	TagDB_free(DATA(self));
}

// -----------------------------------------------------------

IoObject *IoTagDB_setPath(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("setPath(aPath)", "Sets the path to the tag database. Returns self.")
	*/
	IoSeq *path = IoMessage_locals_seqArgAt_(m, locals, 0);
	TagDB_setPath_(DATA(self), CSTRING(path));
	return self;
}

IoObject *IoTagDB_open(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("open", "Open the tagdb. Returns self.")
	*/

	TagDB_open(DATA(self));

	return self;
}

IoObject *IoTagDB_close(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	/*#io
	docSlot("close", "Close the tagdb. Returns self.")
	*/

	TagDB_close(DATA(self));

	return self;
}


Uint64Array *IoTagDB_tagArrayForTagNames_(IoTagDB *self, IoMessage *m, IoList *tagNames)
{
	TagDB *tdb = DATA(self);
	Uint64Array *tags = Uint64Array_new();
	int i;

	for (i = 0; i < IoList_rawSize(tagNames); i ++)
	{
		IoSeq *tagName = IoList_rawAt_(tagNames, i);
		symbolid_t keyid;
		
		IOASSERT(ISSEQ(tagName), "tag names must be Sequences");

		keyid = TagDB_idForSymbol_size_(tdb, CSTRING(tagName), IoSeq_rawSize(tagName));

		Uint64Array_append_(tags, keyid);
		/*
		{
		Datum *keyDatum = TagDB_symbolForId_(tdb, keyid);	
		printf("%s -> %i && ", CSTRING(tagName), (int)keyid);
		printf("%i -> %s\n", (int)keyid, (char *)(keyDatum->data));
		Datum_free(keyDatum);
		}
		*/
	}

	return tags;
}


IoObject *IoTagDB_atKeyPutTags(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	TagDB *tdb = DATA(self);
	IoSeq *key = IoMessage_locals_seqArgAt_(m, locals, 0);
	IoList *tagNames = IoMessage_locals_listArgAt_(m, locals, 1);
	symbolid_t keyid = TagDB_idForSymbol_size_(tdb, CSTRING(key), IoSeq_rawSize(key));
	
	// debugging check
	/*
	Datum *keyDatum = TagDB_symbolForId_(tdb, keyid);
	
	printf("%s -> %i\n", CSTRING(key), (int)keyid);
	printf("%i -> %s\n", (int)keyid, (char *)(keyDatum->data));
	assert(strcmp((char *)(keyDatum->data), (char *)CSTRING(key)) == 0);
	Datum_free(keyDatum);
	*/
	
	{
	Uint64Array *tags = IoTagDB_tagArrayForTagNames_(self, m, tagNames);
	TagDB_begin(tdb);
	TagDB_atKey_putTags_(tdb, keyid, tags);
	TagDB_commit(tdb);
	Uint64Array_free(tags);
	}
	return self;
}

IoObject *IoTagDB_tagsAtKey(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	TagDB *tdb = DATA(self);
	IoSeq *key = IoMessage_locals_seqArgAt_(m, locals, 0);
	symbolid_t keyid = TagDB_idForSymbol_size_(tdb, CSTRING(key), IoSeq_rawSize(key));
	IoList *tagNames = IoList_new(IOSTATE);
	Uint64Array *tags = TagDB_tagsAtKey_(tdb, keyid);
	int i;

	//printf("IoTagDB_tagsAt self = %p\n", (void *)self);

	if (!tags) 
	{
		//printf("IoTagDB_tagsAtKey: no tags found for key\n");
		return IONIL(self);
	}
	
	for (i = 0; i < Uint64Array_size(tags); i ++)
	{
		uint64_t tagid = Uint64Array_at_(tags, i);
		Datum *name = TagDB_symbolForId_(tdb, tagid);
		//printf("tagid %i = %i\n", i, (int)tagid);
		//printf("name '%s'\n", (char *)name->data);

		if (!name)
		{
			printf("IoTagDB_tagsAtKey: no datum returned for TagDB_symbolForId_\n");
		}
		else
		{
			IoList_rawAppend_(tagNames, IoSeq_newWithData_length_(IOSTATE, name->data, name->size));
			Datum_free(name);
		}
	}

	//Uint64Array_free(tags);

	return tagNames;
}

IoObject *IoTagDB_keyAtIndex(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	TagDB *tdb = DATA(self);
	IoNumber *index = IoMessage_locals_numberArgAt_(m, locals, 0);
	Datum *key = TagDB_keyAtIndex_(tdb, CNUMBER(index));
	
	if (!key) 
	{
		return IONIL(self);
	}
	
	return IoSeq_newWithData_length_(IOSTATE, (void *)(key->data), key->size);
}

IoObject *IoTagDB_removeKey(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	TagDB *tdb = DATA(self);
	IoSeq *key = IoMessage_locals_seqArgAt_(m, locals, 0);
	symbolid_t keyid = TagDB_idForSymbol_size_(tdb, CSTRING(key), IoSeq_rawSize(key));
	TagDB_begin(tdb);
	TagDB_removeKey_(tdb, keyid);
	TagDB_commit(tdb);
	return self;
}

IoObject *IoTagDB_keysForTags(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	TagDB *tdb = DATA(self);
	IoList *tagNames = IoMessage_locals_listArgAt_(m, locals, 0);
	Uint64Array *tags = IoTagDB_tagArrayForTagNames_(self, m, tagNames);
	Uint64Array *keys = TagDB_keysForTags_(tdb, tags);
	UArray *keyArray = UArray_newWithData_type_size_copy_(Uint64Array_data(keys), CTYPE_uint64_t, Uint64Array_size(keys), 1);
	IoSeq *keySeq = IoSeq_newWithUArray_copy_(IOSTATE, keyArray, 0);
	Uint64Array_free(tags);
	return keySeq;
}

IoObject *IoTagDB_size(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	return IONUMBER(TagDB_size(DATA(self)));
}

IoObject *IoTagDB_delete(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	TagDB_delete(DATA(self));
	return self;
}

IoObject *IoTagDB_symbolForId(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	symbolid_t id = IoMessage_locals_sizetArgAt_(m, locals, 0);
	Datum *d = TagDB_symbolForId_(DATA(self), id);
	IoSeq *s = IoSeq_newWithData_length_(IOSTATE, Datum_data(d), Datum_size(d));
	Datum_free(d);
	return s;
}

IoObject *IoTagDB_idForSymbol(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	IoSeq *key = IoMessage_locals_seqArgAt_(m, locals, 0);
	return IONUMBER(TagDB_idForSymbol_size_(DATA(self), CSTRING(key), IoSeq_rawSize(key)));
}

IoObject *IoTagDB_allUniqueTagIds(IoTagDB *self, IoObject *locals, IoMessage *m)
{
	Uint64Array *tagIds = TagDB_allUniqueTags(DATA(self)); 
	UArray *ua = UArray_newWithData_type_size_copy_(Uint64Array_data(tagIds), CTYPE_uint64_t, Uint64Array_size(tagIds), 1);
	
	Uint64Array_free(tagIds);	
	return IoSeq_newWithUArray_copy_(IOSTATE, ua, 0);
}

IoTagDB *IoTagDB_proto(void *state)
{
	IoObject *self = IoObject_new(state);
	IoObject_tag_(self, IoTagDB_newTag(state));

	IoState_registerProtoWithFunc_(state, self, IoTagDB_proto);

	{
		IoMethodTable methodTable[] = {
		{"setPath", IoTagDB_setPath},
		{"open", IoTagDB_open},
		{"close", IoTagDB_close},
		{"atKeyPutTags", IoTagDB_atKeyPutTags},
		{"tagsAtKey", IoTagDB_tagsAtKey},
		{"removeKey", IoTagDB_removeKey},
		{"keysForTags", IoTagDB_keysForTags},
		{"size", IoTagDB_size},
		{"symbolForId", IoTagDB_symbolForId},
		{"idForSymbol", IoTagDB_idForSymbol},
		{"keyAtIndex", IoTagDB_keyAtIndex},
		{"delete", IoTagDB_delete},
		{"allUniqueTagIds", IoTagDB_allUniqueTagIds},
		{NULL, NULL},
		};
		IoObject_addMethodTable_(self, methodTable);
	}

	IoObject_setDataPointer_(self, TagDB_new());

	return self;
}
