/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Note that the intrisic @typedArrayLength checks that the argument passed is a typed array
// and throws if it is not.


// Typed Arrays have their own species constructor function since they need
// to look up their default constructor, which is expensive. If we used the
// normal speciesConstructor helper we would need to look up the default
// constructor every time.
@linkTimeConstant
function typedArraySpeciesConstructor(value)
{
    "use strict";
    var constructor = value.constructor;
    if (constructor === @undefined)
        return @typedArrayGetOriginalConstructor(value);

    if (!@isObject(constructor))
        @throwTypeError("|this|.constructor is not an Object or undefined");

    constructor = constructor.@@species;
    if (@isUndefinedOrNull(constructor))
        return @typedArrayGetOriginalConstructor(value);
    // The lack of an @isConstructor(constructor) check here is not observable because
    // the first thing we will do with the value is attempt to construct the result with it.
    // If any user of this function does not immediately construct the result they need to
    // verify that the result is a constructor.
    return constructor;
}

function map(callback /*, thisArg */)
{
    // 22.2.3.18
    "use strict";

    var length = @typedArrayLength(this);

    if (!@isCallable(callback))
        @throwTypeError("TypedArray.prototype.map callback must be a function");

    var thisArg = @argument(1);

    var constructor = @typedArraySpeciesConstructor(this);
    var result = new constructor(length);
    if (@typedArrayLength(result) < length)
        @throwTypeError("TypedArray.prototype.map constructed typed array of insufficient length");
    if (@typedArrayContentType(this) !== @typedArrayContentType(result))
        @throwTypeError("TypedArray.prototype.map constructed typed array of different content type from |this|");

    for (var i = 0; i < length; i++) {
        var mappedValue = callback.@call(thisArg, this[i], i, this);
        result[i] = mappedValue;
    }
    return result;
}

function filter(callback /*, thisArg */)
{
    "use strict";

    var length = @typedArrayLength(this);

    if (!@isCallable(callback))
        @throwTypeError("TypedArray.prototype.filter callback must be a function");

    var thisArg = @argument(1);
    var kept = [];

    for (var i = 0; i < length; i++) {
        var value = this[i];
        if (callback.@call(thisArg, value, i, this))
            @arrayPush(kept, value);
    }
    var length = kept.length;

    var constructor = @typedArraySpeciesConstructor(this);
    var result = new constructor(length);
    if (@typedArrayLength(result) < length)
        @throwTypeError("TypedArray.prototype.filter constructed typed array of insufficient length");
    if (@typedArrayContentType(this) !== @typedArrayContentType(result))
        @throwTypeError("TypedArray.prototype.filter constructed typed array of different content type from |this|");

    for (var i = 0; i < length; i++)
        result[i] = kept[i];

    return result;
}

function toLocaleString(/* locale, options */)
{
    "use strict";

    var length = @typedArrayLength(this);

    if (length == 0)
        return "";

    var string = "";
    for (var i = 0; i < length; ++i) {
        if (i > 0)
            string += ",";
        var element = this[i];
        if (!@isUndefinedOrNull(element))
            string += @toString(element.toLocaleString(@argument(0), @argument(1)));
    }

    return string;
}

function at(index)
{
    "use strict";

    var length = @typedArrayLength(this);

    var k = @toIntegerOrInfinity(index);
    if (k < 0)
        k += length;

    return (k >= 0 && k < length) ? this[k] : @undefined;
}
