

Obsidian := Object clone do(
	name ::= "test"
	sharedPrefixCursor ::= nil
	path ::= nil
	
	init := method(	
		self db := TokyoCabinet clone
	)
	
	open := method(
		db open(path)
		setSharedPrefixCursor(db prefixCursor)
		writeln("db open")
		self
	)
	
	close := method(
		self sharedPrefixCursor := nil
		db close
		self
	)
	
	delete := method(
		db close
		File with(path) remove
		self
	)
	
	onAtPut := method(objId, slotName, value,
		writeln("onAtPut(",objId, ", ", slotName, ", ", value, ")")
		db transactionalAtPut(objId .. "/" .. slotName, value)
	)
	
	onAt := method(objId, slotName,
		db at(objId .. "/" .. slotName)
	)
	
	onRemoveAt := method(objId, slotName,
		db transactionalRemoveAt(objId .. "/" .. slotName)
		nil
	)
	
	cursorOn := method(objId,
		db prefixCursor setPrefix(objId)
	)
	
	// network API
	
	acceptedMessageNames := list("onAtPut", "onAt", "onRemoveAt", "onFirst", "onLast", "onAfter", "onBefore")
	
	onFirst := method(objId, count,
		c := sharedPrefixCursor setPrefix(objId asString)
		c first
		keys := list()
		count asNumber repeat(
			k := c key
			if(k == nil, break)
			keys append(k)
			c next
		)
		keys	
	)
				
	onLast := method(objId, count,
		c := sharedPrefixCursor setPrefix(objId)
		c last
		keys := list()
		arg1 asNumber repeat(
			k := c key
			if(k == nil, break)
			keys append(k)
			c previous
		)
		keys			
	)
			
	onAfter := method(objId, slotName, count,
		c := sharedPrefixCursor setPrefix(objId)
		c goto(slotName)
		keys := list()
		count asNumber repeat(
			k := c key
			if(k == nil, break)
			keys append(k)
			c next
		)
		keys			
	)

	before := method(objId, slotName, count,
		c := sharedPrefixCursor setPrefix(objId)
		c goto(slotName)
		keys := list()
		count asNumber repeat(
			k := c key
			if(k == nil, break)
			keys append(k)
			c previous
		)
		keys			
	)	
)
