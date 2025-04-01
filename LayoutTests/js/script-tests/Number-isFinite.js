description("Test the basic behaviors of Number.isFinite()");

shouldBeTrue('Number.hasOwnProperty("isFinite")');
shouldBeEqualToString('typeof Number.isFinite', 'function');
shouldBeTrue('Number.isFinite !== isFinite');

// Function properties.
shouldBe('Number.isFinite.length', '1');
shouldBeEqualToString('Number.isFinite.name', 'isFinite');
shouldBe('Object.getOwnPropertyDescriptor(Number, "isFinite").configurable', 'true');
shouldBe('Object.getOwnPropertyDescriptor(Number, "isFinite").enumerable', 'false');
shouldBe('Object.getOwnPropertyDescriptor(Number, "isFinite").writable', 'true');

// Some simple cases.
shouldBeFalse('Number.isFinite()');
shouldBeFalse('Number.isFinite(NaN)');
shouldBeFalse('Number.isFinite(undefined)');

shouldBeTrue('Number.isFinite(0)');
shouldBeTrue('Number.isFinite(-0)');
shouldBeTrue('Number.isFinite(1)');
shouldBeTrue('Number.isFinite(-1)');
shouldBeTrue('Number.isFinite(42)');
shouldBeTrue('Number.isFinite(123.5)');
shouldBeTrue('Number.isFinite(-123.5)');
shouldBeTrue('Number.isFinite(1e10)');
shouldBeTrue('Number.isFinite(-1e10)');
shouldBeTrue('Number.isFinite(1.7e10)');
shouldBeTrue('Number.isFinite(-1.7e10)');

shouldBeTrue('Number.isFinite(Number.MAX_VALUE)');
shouldBeTrue('Number.isFinite(Number.MIN_VALUE)');
shouldBeTrue('Number.isFinite(Number.MAX_SAFE_INTEGER)');
shouldBeTrue('Number.isFinite(Number.MIN_SAFE_INTEGER)');
shouldBeTrue('Number.isFinite(Math.PI)');
shouldBeTrue('Number.isFinite(Math.E)');
shouldBeTrue('Number.isFinite(Math.LOG2E)');
shouldBeTrue('Number.isFinite(Math.LOG10E)');
shouldBeFalse('Number.isFinite(Infinity)');
shouldBeFalse('Number.isFinite(-Infinity)');
shouldBeFalse('Number.isFinite(null)');
shouldBeTrue('Number.isFinite(1, 3)');
shouldBeFalse('Number.isFinite(Infinity, 3)');

// Non-numeric.
shouldBeFalse('Number.isFinite({})');
shouldBeFalse('Number.isFinite({ webkit: "awesome" })');
shouldBeFalse('Number.isFinite([])');
shouldBeFalse('Number.isFinite([123])');
shouldBeFalse('Number.isFinite([1,1])');
shouldBeFalse('Number.isFinite([NaN])');
shouldBeFalse('Number.isFinite("")');
shouldBeFalse('Number.isFinite("1")');
shouldBeFalse('Number.isFinite("x")');
shouldBeFalse('Number.isFinite("NaN")');
shouldBeFalse('Number.isFinite("Infinity")');
shouldBeFalse('Number.isFinite(true)');
shouldBeFalse('Number.isFinite(false)');
shouldBeFalse('Number.isFinite(function(){})');
shouldBeFalse('Number.isFinite(()=>{})');
shouldBeFalse('Number.isFinite(isFinite)');
shouldBeFalse('Number.isFinite(Symbol())');
shouldBeFalse('Number.isFinite(new Number(123))');
shouldBeFalse('Number.isFinite(new Number(-123))');
shouldBeFalse('Number.isFinite(new Number("123"))');
shouldBeFalse('Number.isFinite(new Number(undefined))');
shouldBeFalse('Number.isFinite(new Number(true))');
shouldBeFalse('Number.isFinite(new Number(false))');
shouldBeFalse('Number.isFinite(BigInt(123))');
shouldBeFalse('Number.isFinite(BigInt(-123))');
shouldBeFalse('Number.isFinite(BigInt("123"))');
shouldBeFalse('Number.isFinite(BigInt("-123"))');
shouldBeFalse('Number.isFinite(BigInt("0x1fffffffffffff"))');
shouldBeFalse('Number.isFinite(BigInt("0o377777777777777777"))');

// Type conversion, doesn't happen.
var objectWithNumberValueOf = { valueOf: function() { return 123; } };
var objectWithNaNValueOf = { valueOf: function() { return NaN; } };
shouldBeFalse('Number.isFinite(objectWithNumberValueOf)');
shouldBeFalse('Number.isFinite(objectWithNaNValueOf)');

var objectRecordConversionCalls = {
    callList: [],
    toString: function() {
        this.callList.push("toString");
        return "Bad";
    },
    valueOf: function() {
        this.callList.push("valueOf");
        return 12345;
    }
};
shouldBeFalse('Number.isFinite(objectRecordConversionCalls)');
shouldBe('objectRecordConversionCalls.callList.length', '0');
