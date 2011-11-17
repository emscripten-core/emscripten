
function JSUint64()
{
	this.high = 0;
	this.low = 0;
	
	setBit: function(index)
	{
		if( index < 32 )
			low = low | (1 << index);
		else
			high = high | (1 << index);
	};
	
	bitAnd: function(other)
	{
		low  |= other.low;
		high |= other.high;
	};
	
	bitOr: function(other)
	{
		low  &= other.low;
		high &= other.high;
	};
	
	leftShiftBy: function(bit_count)
	{
	};
	
	rightShiftBy: function(bit_count)
	{
	};
	
	leftRotateBy: function(bit_count)
	{
		high = ((hi << bit_count) | (low >>> (32 - bit_count))) & (0xFFFFFFFF)
		low  = ((lo << bit_count) | (high >>> (32 - bit_count))) & (0xFFFFFFFF)
	};
	
	rightRotateBy: function(bit_count)
	{
		high = ((hi >> bit_count) | (low <<< (32 - bit_count))) & (0xFFFFFFFF)
		low  = ((lo >> bit_count) | (high <<< (32 - bit_count))) & (0xFFFFFFFF)
	};
}

jsuint64 = 
{
	_outsideMap: [],
	_freeIndex: [],
	'jsuint64_create': function()
	{
		var jsuint = new JSUint64();
		
		if( _freeIndex.length != 0 )
			return _outsideMap.push(jsuint);
		
		var index = _freeIndex.pop();
		_outsideMap[index] = jsuint;
		return index;
	},
	
	'jsuint64_setbit': function(jsuint, index)
	{
		_outsideMap[jsuint].setBit(index);
	},
	
	'jsuint64_bitAnd': function(jsuint, other)
	{
		_outsideMap[jsuint].bitAnd(_outsideMap[other]);
	},
	
	'jsuint64_bitOr': function(jsuint, other)
	{
		_outsideMap[jsuint].bitOr(_outsideMap[other]);
	},
	
	'jsuint64_isSet': function(jsuint)
	{
		return _outsideMap[jsuint].isSet();
	},
	
	'jsuint64_destroy': function(jsuint)
	{
		var index = _outsideMap[jsuint];
		
		_freeIndex.push(index);
		delete _outsideMap[jsuint];
	},
};
