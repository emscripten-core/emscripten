(function (global, factory) {
typeof exports === 'object' && typeof module !== 'undefined' ? factory(exports) :
typeof define === 'function' && define.amd ? define(['exports'], factory) :
(global = global || self, factory(global.Terser = {}));
}(this, (function (exports) { 'use strict';

/***********************************************************************

  A JavaScript tokenizer / parser / beautifier / compressor.
  https://github.com/mishoo/UglifyJS2

  -------------------------------- (C) ---------------------------------

                           Author: Mihai Bazon
                         <mihai.bazon@gmail.com>
                       http://mihai.bazon.net/blog

  Distributed under the BSD license:

    Copyright 2012 (c) Mihai Bazon <mihai.bazon@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        * Redistributions of source code must retain the above
          copyright notice, this list of conditions and the following
          disclaimer.

        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials
          provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

 ***********************************************************************/

function characters(str) {
    return str.split("");
}

class DefaultsError extends Error {
    constructor(msg, defs) {
        super();

        this.name = "DefaultsError";
        this.message = msg;
        this.defs = defs;
    }
}

function defaults(args, defs, croak) {
    if (args === true) {
        args = {};
    } else if (args != null && typeof args === "object") {
        args = {...args};
    }

    const ret = args || {};

    if (croak) for (const i in ret) if (HOP(ret, i) && !HOP(defs, i)) {
        throw new DefaultsError("`" + i + "` is not a supported option", defs);
    }

    for (const i in defs) if (HOP(defs, i)) {
        if (!args || !HOP(args, i)) {
            ret[i] = defs[i];
        } else if (i === "ecma") {
            let ecma = args[i] | 0;
            if (ecma > 5 && ecma < 2015) ecma += 2009;
            ret[i] = ecma;
        } else {
            ret[i] = (args && HOP(args, i)) ? args[i] : defs[i];
        }
    }

    return ret;
}

function noop() {}
function return_false() { return false; }
function return_true() { return true; }
function return_this() { return this; }

var MAP = (function() {
    function MAP(a, tw, allow_splicing = true) {
        const new_a = [];

        for (let i = 0; i < a.length; ++i) {
            let item = a[i];
            let ret = item.transform(tw, allow_splicing);

            if (ret instanceof AST_Node) {
                new_a.push(ret);
            } else if (ret instanceof Splice) {
                new_a.push(...ret.v);
            }
        }

        return new_a;
    }

    MAP.splice = function(val) { return new Splice(val); };
    MAP.skip = {};
    function Splice(val) { this.v = val; }
    return MAP;
})();

function make_node(ctor, orig, props) {
    if (!props) props = {};
    if (orig) {
        if (!props.start) props.start = orig.start;
        if (!props.end) props.end = orig.end;
    }
    return new ctor(props);
}

function push_uniq(array, el) {
    if (!array.includes(el))
        array.push(el);
}

function string_template(text, props) {
    return text.replace(/{(.+?)}/g, function(str, p) {
        return props && props[p];
    });
}

function mergeSort(array, cmp) {
    if (array.length < 2) return array.slice();
    function merge(a, b) {
        var r = [], ai = 0, bi = 0, i = 0;
        while (ai < a.length && bi < b.length) {
            cmp(a[ai], b[bi]) <= 0
                ? r[i++] = a[ai++]
                : r[i++] = b[bi++];
        }
        if (ai < a.length) r.push.apply(r, a.slice(ai));
        if (bi < b.length) r.push.apply(r, b.slice(bi));
        return r;
    }
    function _ms(a) {
        if (a.length <= 1)
            return a;
        var m = Math.floor(a.length / 2), left = a.slice(0, m), right = a.slice(m);
        left = _ms(left);
        right = _ms(right);
        return merge(left, right);
    }
    return _ms(array);
}

function makePredicate(words) {
    if (!Array.isArray(words)) words = words.split(" ");

    return new Set(words.sort());
}

function HOP(obj, prop) {
    return Object.prototype.hasOwnProperty.call(obj, prop);
}

function keep_name(keep_setting, name) {
    return keep_setting === true
        || (keep_setting instanceof RegExp && keep_setting.test(name));
}

var lineTerminatorEscape = {
    "\0": "0",
    "\n": "n",
    "\r": "r",
    "\u2028": "u2028",
    "\u2029": "u2029",
};
function regexp_source_fix(source) {
    // V8 does not escape line terminators in regexp patterns in node 12
    // We'll also remove literal \0
    return source.replace(/[\0\n\r\u2028\u2029]/g, function (match, offset) {
        var escaped = source[offset - 1] == "\\"
            && (source[offset - 2] != "\\"
            || /(?:^|[^\\])(?:\\{2})*$/.test(source.slice(0, offset - 1)));
        return (escaped ? "" : "\\") + lineTerminatorEscape[match];
    });
}

const all_flags = "dgimsuyv";
function sort_regexp_flags(flags) {
    const existing_flags = new Set(flags.split(""));
    let out = "";
    for (const flag of all_flags) {
        if (existing_flags.has(flag)) {
            out += flag;
            existing_flags.delete(flag);
        }
    }
    if (existing_flags.size) {
        // Flags Terser doesn't know about
        existing_flags.forEach(flag => { out += flag; });
    }
    return out;
}

function set_annotation(node, annotation) {
    node._annotations |= annotation;
}

/***********************************************************************

  A JavaScript tokenizer / parser / beautifier / compressor.
  https://github.com/mishoo/UglifyJS2

  -------------------------------- (C) ---------------------------------

                           Author: Mihai Bazon
                         <mihai.bazon@gmail.com>
                       http://mihai.bazon.net/blog

  Distributed under the BSD license:

    Copyright 2012 (c) Mihai Bazon <mihai.bazon@gmail.com>
    Parser based on parse-js (http://marijn.haverbeke.nl/parse-js/).

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        * Redistributions of source code must retain the above
          copyright notice, this list of conditions and the following
          disclaimer.

        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials
          provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

 ***********************************************************************/

var LATEST_RAW = "";  // Only used for numbers and template strings
var TEMPLATE_RAWS = new Map();  // Raw template strings

var KEYWORDS = "break case catch class const continue debugger default delete do else export extends finally for function if in instanceof let new return switch throw try typeof var void while with";
var KEYWORDS_ATOM = "false null true";
var RESERVED_WORDS = "enum import super this " + KEYWORDS_ATOM + " " + KEYWORDS;
var ALL_RESERVED_WORDS = "implements interface package private protected public static " + RESERVED_WORDS;
var KEYWORDS_BEFORE_EXPRESSION = "return new delete throw else case yield await";

KEYWORDS = makePredicate(KEYWORDS);
RESERVED_WORDS = makePredicate(RESERVED_WORDS);
KEYWORDS_BEFORE_EXPRESSION = makePredicate(KEYWORDS_BEFORE_EXPRESSION);
KEYWORDS_ATOM = makePredicate(KEYWORDS_ATOM);
ALL_RESERVED_WORDS = makePredicate(ALL_RESERVED_WORDS);

var OPERATOR_CHARS = makePredicate(characters("+-*&%=<>!?|~^"));

var RE_NUM_LITERAL = /[0-9a-f]/i;
var RE_HEX_NUMBER = /^0x[0-9a-f]+$/i;
var RE_OCT_NUMBER = /^0[0-7]+$/;
var RE_ES6_OCT_NUMBER = /^0o[0-7]+$/i;
var RE_BIN_NUMBER = /^0b[01]+$/i;
var RE_DEC_NUMBER = /^\d*\.?\d*(?:e[+-]?\d*(?:\d\.?|\.?\d)\d*)?$/i;
var RE_BIG_INT = /^(0[xob])?[0-9a-f]+n$/i;

var OPERATORS = makePredicate([
    "in",
    "instanceof",
    "typeof",
    "new",
    "void",
    "delete",
    "++",
    "--",
    "+",
    "-",
    "!",
    "~",
    "&",
    "|",
    "^",
    "*",
    "**",
    "/",
    "%",
    ">>",
    "<<",
    ">>>",
    "<",
    ">",
    "<=",
    ">=",
    "==",
    "===",
    "!=",
    "!==",
    "?",
    "=",
    "+=",
    "-=",
    "||=",
    "&&=",
    "??=",
    "/=",
    "*=",
    "**=",
    "%=",
    ">>=",
    "<<=",
    ">>>=",
    "|=",
    "^=",
    "&=",
    "&&",
    "??",
    "||",
]);

var WHITESPACE_CHARS = makePredicate(characters(" \u00a0\n\r\t\f\u000b\u200b\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u2028\u2029\u202f\u205f\u3000\uFEFF"));

var NEWLINE_CHARS = makePredicate(characters("\n\r\u2028\u2029"));

var PUNC_AFTER_EXPRESSION = makePredicate(characters(";]),:"));

var PUNC_BEFORE_EXPRESSION = makePredicate(characters("[{(,;:"));

var PUNC_CHARS = makePredicate(characters("[]{}(),;:"));

/* -----[ Tokenizer ]----- */

// surrogate safe regexps adapted from https://github.com/mathiasbynens/unicode-8.0.0/tree/89b412d8a71ecca9ed593d9e9fa073ab64acfebe/Binary_Property
var UNICODE = {
    ID_Start: /[$A-Z_a-z\xAA\xB5\xBA\xC0-\xD6\xD8-\xF6\xF8-\u02C1\u02C6-\u02D1\u02E0-\u02E4\u02EC\u02EE\u0370-\u0374\u0376\u0377\u037A-\u037D\u037F\u0386\u0388-\u038A\u038C\u038E-\u03A1\u03A3-\u03F5\u03F7-\u0481\u048A-\u052F\u0531-\u0556\u0559\u0561-\u0587\u05D0-\u05EA\u05F0-\u05F2\u0620-\u064A\u066E\u066F\u0671-\u06D3\u06D5\u06E5\u06E6\u06EE\u06EF\u06FA-\u06FC\u06FF\u0710\u0712-\u072F\u074D-\u07A5\u07B1\u07CA-\u07EA\u07F4\u07F5\u07FA\u0800-\u0815\u081A\u0824\u0828\u0840-\u0858\u08A0-\u08B4\u0904-\u0939\u093D\u0950\u0958-\u0961\u0971-\u0980\u0985-\u098C\u098F\u0990\u0993-\u09A8\u09AA-\u09B0\u09B2\u09B6-\u09B9\u09BD\u09CE\u09DC\u09DD\u09DF-\u09E1\u09F0\u09F1\u0A05-\u0A0A\u0A0F\u0A10\u0A13-\u0A28\u0A2A-\u0A30\u0A32\u0A33\u0A35\u0A36\u0A38\u0A39\u0A59-\u0A5C\u0A5E\u0A72-\u0A74\u0A85-\u0A8D\u0A8F-\u0A91\u0A93-\u0AA8\u0AAA-\u0AB0\u0AB2\u0AB3\u0AB5-\u0AB9\u0ABD\u0AD0\u0AE0\u0AE1\u0AF9\u0B05-\u0B0C\u0B0F\u0B10\u0B13-\u0B28\u0B2A-\u0B30\u0B32\u0B33\u0B35-\u0B39\u0B3D\u0B5C\u0B5D\u0B5F-\u0B61\u0B71\u0B83\u0B85-\u0B8A\u0B8E-\u0B90\u0B92-\u0B95\u0B99\u0B9A\u0B9C\u0B9E\u0B9F\u0BA3\u0BA4\u0BA8-\u0BAA\u0BAE-\u0BB9\u0BD0\u0C05-\u0C0C\u0C0E-\u0C10\u0C12-\u0C28\u0C2A-\u0C39\u0C3D\u0C58-\u0C5A\u0C60\u0C61\u0C85-\u0C8C\u0C8E-\u0C90\u0C92-\u0CA8\u0CAA-\u0CB3\u0CB5-\u0CB9\u0CBD\u0CDE\u0CE0\u0CE1\u0CF1\u0CF2\u0D05-\u0D0C\u0D0E-\u0D10\u0D12-\u0D3A\u0D3D\u0D4E\u0D5F-\u0D61\u0D7A-\u0D7F\u0D85-\u0D96\u0D9A-\u0DB1\u0DB3-\u0DBB\u0DBD\u0DC0-\u0DC6\u0E01-\u0E30\u0E32\u0E33\u0E40-\u0E46\u0E81\u0E82\u0E84\u0E87\u0E88\u0E8A\u0E8D\u0E94-\u0E97\u0E99-\u0E9F\u0EA1-\u0EA3\u0EA5\u0EA7\u0EAA\u0EAB\u0EAD-\u0EB0\u0EB2\u0EB3\u0EBD\u0EC0-\u0EC4\u0EC6\u0EDC-\u0EDF\u0F00\u0F40-\u0F47\u0F49-\u0F6C\u0F88-\u0F8C\u1000-\u102A\u103F\u1050-\u1055\u105A-\u105D\u1061\u1065\u1066\u106E-\u1070\u1075-\u1081\u108E\u10A0-\u10C5\u10C7\u10CD\u10D0-\u10FA\u10FC-\u1248\u124A-\u124D\u1250-\u1256\u1258\u125A-\u125D\u1260-\u1288\u128A-\u128D\u1290-\u12B0\u12B2-\u12B5\u12B8-\u12BE\u12C0\u12C2-\u12C5\u12C8-\u12D6\u12D8-\u1310\u1312-\u1315\u1318-\u135A\u1380-\u138F\u13A0-\u13F5\u13F8-\u13FD\u1401-\u166C\u166F-\u167F\u1681-\u169A\u16A0-\u16EA\u16EE-\u16F8\u1700-\u170C\u170E-\u1711\u1720-\u1731\u1740-\u1751\u1760-\u176C\u176E-\u1770\u1780-\u17B3\u17D7\u17DC\u1820-\u1877\u1880-\u18A8\u18AA\u18B0-\u18F5\u1900-\u191E\u1950-\u196D\u1970-\u1974\u1980-\u19AB\u19B0-\u19C9\u1A00-\u1A16\u1A20-\u1A54\u1AA7\u1B05-\u1B33\u1B45-\u1B4B\u1B83-\u1BA0\u1BAE\u1BAF\u1BBA-\u1BE5\u1C00-\u1C23\u1C4D-\u1C4F\u1C5A-\u1C7D\u1CE9-\u1CEC\u1CEE-\u1CF1\u1CF5\u1CF6\u1D00-\u1DBF\u1E00-\u1F15\u1F18-\u1F1D\u1F20-\u1F45\u1F48-\u1F4D\u1F50-\u1F57\u1F59\u1F5B\u1F5D\u1F5F-\u1F7D\u1F80-\u1FB4\u1FB6-\u1FBC\u1FBE\u1FC2-\u1FC4\u1FC6-\u1FCC\u1FD0-\u1FD3\u1FD6-\u1FDB\u1FE0-\u1FEC\u1FF2-\u1FF4\u1FF6-\u1FFC\u2071\u207F\u2090-\u209C\u2102\u2107\u210A-\u2113\u2115\u2118-\u211D\u2124\u2126\u2128\u212A-\u2139\u213C-\u213F\u2145-\u2149\u214E\u2160-\u2188\u2C00-\u2C2E\u2C30-\u2C5E\u2C60-\u2CE4\u2CEB-\u2CEE\u2CF2\u2CF3\u2D00-\u2D25\u2D27\u2D2D\u2D30-\u2D67\u2D6F\u2D80-\u2D96\u2DA0-\u2DA6\u2DA8-\u2DAE\u2DB0-\u2DB6\u2DB8-\u2DBE\u2DC0-\u2DC6\u2DC8-\u2DCE\u2DD0-\u2DD6\u2DD8-\u2DDE\u3005-\u3007\u3021-\u3029\u3031-\u3035\u3038-\u303C\u3041-\u3096\u309B-\u309F\u30A1-\u30FA\u30FC-\u30FF\u3105-\u312D\u3131-\u318E\u31A0-\u31BA\u31F0-\u31FF\u3400-\u4DB5\u4E00-\u9FD5\uA000-\uA48C\uA4D0-\uA4FD\uA500-\uA60C\uA610-\uA61F\uA62A\uA62B\uA640-\uA66E\uA67F-\uA69D\uA6A0-\uA6EF\uA717-\uA71F\uA722-\uA788\uA78B-\uA7AD\uA7B0-\uA7B7\uA7F7-\uA801\uA803-\uA805\uA807-\uA80A\uA80C-\uA822\uA840-\uA873\uA882-\uA8B3\uA8F2-\uA8F7\uA8FB\uA8FD\uA90A-\uA925\uA930-\uA946\uA960-\uA97C\uA984-\uA9B2\uA9CF\uA9E0-\uA9E4\uA9E6-\uA9EF\uA9FA-\uA9FE\uAA00-\uAA28\uAA40-\uAA42\uAA44-\uAA4B\uAA60-\uAA76\uAA7A\uAA7E-\uAAAF\uAAB1\uAAB5\uAAB6\uAAB9-\uAABD\uAAC0\uAAC2\uAADB-\uAADD\uAAE0-\uAAEA\uAAF2-\uAAF4\uAB01-\uAB06\uAB09-\uAB0E\uAB11-\uAB16\uAB20-\uAB26\uAB28-\uAB2E\uAB30-\uAB5A\uAB5C-\uAB65\uAB70-\uABE2\uAC00-\uD7A3\uD7B0-\uD7C6\uD7CB-\uD7FB\uF900-\uFA6D\uFA70-\uFAD9\uFB00-\uFB06\uFB13-\uFB17\uFB1D\uFB1F-\uFB28\uFB2A-\uFB36\uFB38-\uFB3C\uFB3E\uFB40\uFB41\uFB43\uFB44\uFB46-\uFBB1\uFBD3-\uFD3D\uFD50-\uFD8F\uFD92-\uFDC7\uFDF0-\uFDFB\uFE70-\uFE74\uFE76-\uFEFC\uFF21-\uFF3A\uFF41-\uFF5A\uFF66-\uFFBE\uFFC2-\uFFC7\uFFCA-\uFFCF\uFFD2-\uFFD7\uFFDA-\uFFDC]|\uD800[\uDC00-\uDC0B\uDC0D-\uDC26\uDC28-\uDC3A\uDC3C\uDC3D\uDC3F-\uDC4D\uDC50-\uDC5D\uDC80-\uDCFA\uDD40-\uDD74\uDE80-\uDE9C\uDEA0-\uDED0\uDF00-\uDF1F\uDF30-\uDF4A\uDF50-\uDF75\uDF80-\uDF9D\uDFA0-\uDFC3\uDFC8-\uDFCF\uDFD1-\uDFD5]|\uD801[\uDC00-\uDC9D\uDD00-\uDD27\uDD30-\uDD63\uDE00-\uDF36\uDF40-\uDF55\uDF60-\uDF67]|\uD802[\uDC00-\uDC05\uDC08\uDC0A-\uDC35\uDC37\uDC38\uDC3C\uDC3F-\uDC55\uDC60-\uDC76\uDC80-\uDC9E\uDCE0-\uDCF2\uDCF4\uDCF5\uDD00-\uDD15\uDD20-\uDD39\uDD80-\uDDB7\uDDBE\uDDBF\uDE00\uDE10-\uDE13\uDE15-\uDE17\uDE19-\uDE33\uDE60-\uDE7C\uDE80-\uDE9C\uDEC0-\uDEC7\uDEC9-\uDEE4\uDF00-\uDF35\uDF40-\uDF55\uDF60-\uDF72\uDF80-\uDF91]|\uD803[\uDC00-\uDC48\uDC80-\uDCB2\uDCC0-\uDCF2]|\uD804[\uDC03-\uDC37\uDC83-\uDCAF\uDCD0-\uDCE8\uDD03-\uDD26\uDD50-\uDD72\uDD76\uDD83-\uDDB2\uDDC1-\uDDC4\uDDDA\uDDDC\uDE00-\uDE11\uDE13-\uDE2B\uDE80-\uDE86\uDE88\uDE8A-\uDE8D\uDE8F-\uDE9D\uDE9F-\uDEA8\uDEB0-\uDEDE\uDF05-\uDF0C\uDF0F\uDF10\uDF13-\uDF28\uDF2A-\uDF30\uDF32\uDF33\uDF35-\uDF39\uDF3D\uDF50\uDF5D-\uDF61]|\uD805[\uDC80-\uDCAF\uDCC4\uDCC5\uDCC7\uDD80-\uDDAE\uDDD8-\uDDDB\uDE00-\uDE2F\uDE44\uDE80-\uDEAA\uDF00-\uDF19]|\uD806[\uDCA0-\uDCDF\uDCFF\uDEC0-\uDEF8]|\uD808[\uDC00-\uDF99]|\uD809[\uDC00-\uDC6E\uDC80-\uDD43]|[\uD80C\uD840-\uD868\uD86A-\uD86C\uD86F-\uD872][\uDC00-\uDFFF]|\uD80D[\uDC00-\uDC2E]|\uD811[\uDC00-\uDE46]|\uD81A[\uDC00-\uDE38\uDE40-\uDE5E\uDED0-\uDEED\uDF00-\uDF2F\uDF40-\uDF43\uDF63-\uDF77\uDF7D-\uDF8F]|\uD81B[\uDF00-\uDF44\uDF50\uDF93-\uDF9F]|\uD82C[\uDC00\uDC01]|\uD82F[\uDC00-\uDC6A\uDC70-\uDC7C\uDC80-\uDC88\uDC90-\uDC99]|\uD835[\uDC00-\uDC54\uDC56-\uDC9C\uDC9E\uDC9F\uDCA2\uDCA5\uDCA6\uDCA9-\uDCAC\uDCAE-\uDCB9\uDCBB\uDCBD-\uDCC3\uDCC5-\uDD05\uDD07-\uDD0A\uDD0D-\uDD14\uDD16-\uDD1C\uDD1E-\uDD39\uDD3B-\uDD3E\uDD40-\uDD44\uDD46\uDD4A-\uDD50\uDD52-\uDEA5\uDEA8-\uDEC0\uDEC2-\uDEDA\uDEDC-\uDEFA\uDEFC-\uDF14\uDF16-\uDF34\uDF36-\uDF4E\uDF50-\uDF6E\uDF70-\uDF88\uDF8A-\uDFA8\uDFAA-\uDFC2\uDFC4-\uDFCB]|\uD83A[\uDC00-\uDCC4]|\uD83B[\uDE00-\uDE03\uDE05-\uDE1F\uDE21\uDE22\uDE24\uDE27\uDE29-\uDE32\uDE34-\uDE37\uDE39\uDE3B\uDE42\uDE47\uDE49\uDE4B\uDE4D-\uDE4F\uDE51\uDE52\uDE54\uDE57\uDE59\uDE5B\uDE5D\uDE5F\uDE61\uDE62\uDE64\uDE67-\uDE6A\uDE6C-\uDE72\uDE74-\uDE77\uDE79-\uDE7C\uDE7E\uDE80-\uDE89\uDE8B-\uDE9B\uDEA1-\uDEA3\uDEA5-\uDEA9\uDEAB-\uDEBB]|\uD869[\uDC00-\uDED6\uDF00-\uDFFF]|\uD86D[\uDC00-\uDF34\uDF40-\uDFFF]|\uD86E[\uDC00-\uDC1D\uDC20-\uDFFF]|\uD873[\uDC00-\uDEA1]|\uD87E[\uDC00-\uDE1D]/,
    ID_Continue: /(?:[$0-9A-Z_a-z\xAA\xB5\xB7\xBA\xC0-\xD6\xD8-\xF6\xF8-\u02C1\u02C6-\u02D1\u02E0-\u02E4\u02EC\u02EE\u0300-\u0374\u0376\u0377\u037A-\u037D\u037F\u0386-\u038A\u038C\u038E-\u03A1\u03A3-\u03F5\u03F7-\u0481\u0483-\u0487\u048A-\u052F\u0531-\u0556\u0559\u0561-\u0587\u0591-\u05BD\u05BF\u05C1\u05C2\u05C4\u05C5\u05C7\u05D0-\u05EA\u05F0-\u05F2\u0610-\u061A\u0620-\u0669\u066E-\u06D3\u06D5-\u06DC\u06DF-\u06E8\u06EA-\u06FC\u06FF\u0710-\u074A\u074D-\u07B1\u07C0-\u07F5\u07FA\u0800-\u082D\u0840-\u085B\u08A0-\u08B4\u08E3-\u0963\u0966-\u096F\u0971-\u0983\u0985-\u098C\u098F\u0990\u0993-\u09A8\u09AA-\u09B0\u09B2\u09B6-\u09B9\u09BC-\u09C4\u09C7\u09C8\u09CB-\u09CE\u09D7\u09DC\u09DD\u09DF-\u09E3\u09E6-\u09F1\u0A01-\u0A03\u0A05-\u0A0A\u0A0F\u0A10\u0A13-\u0A28\u0A2A-\u0A30\u0A32\u0A33\u0A35\u0A36\u0A38\u0A39\u0A3C\u0A3E-\u0A42\u0A47\u0A48\u0A4B-\u0A4D\u0A51\u0A59-\u0A5C\u0A5E\u0A66-\u0A75\u0A81-\u0A83\u0A85-\u0A8D\u0A8F-\u0A91\u0A93-\u0AA8\u0AAA-\u0AB0\u0AB2\u0AB3\u0AB5-\u0AB9\u0ABC-\u0AC5\u0AC7-\u0AC9\u0ACB-\u0ACD\u0AD0\u0AE0-\u0AE3\u0AE6-\u0AEF\u0AF9\u0B01-\u0B03\u0B05-\u0B0C\u0B0F\u0B10\u0B13-\u0B28\u0B2A-\u0B30\u0B32\u0B33\u0B35-\u0B39\u0B3C-\u0B44\u0B47\u0B48\u0B4B-\u0B4D\u0B56\u0B57\u0B5C\u0B5D\u0B5F-\u0B63\u0B66-\u0B6F\u0B71\u0B82\u0B83\u0B85-\u0B8A\u0B8E-\u0B90\u0B92-\u0B95\u0B99\u0B9A\u0B9C\u0B9E\u0B9F\u0BA3\u0BA4\u0BA8-\u0BAA\u0BAE-\u0BB9\u0BBE-\u0BC2\u0BC6-\u0BC8\u0BCA-\u0BCD\u0BD0\u0BD7\u0BE6-\u0BEF\u0C00-\u0C03\u0C05-\u0C0C\u0C0E-\u0C10\u0C12-\u0C28\u0C2A-\u0C39\u0C3D-\u0C44\u0C46-\u0C48\u0C4A-\u0C4D\u0C55\u0C56\u0C58-\u0C5A\u0C60-\u0C63\u0C66-\u0C6F\u0C81-\u0C83\u0C85-\u0C8C\u0C8E-\u0C90\u0C92-\u0CA8\u0CAA-\u0CB3\u0CB5-\u0CB9\u0CBC-\u0CC4\u0CC6-\u0CC8\u0CCA-\u0CCD\u0CD5\u0CD6\u0CDE\u0CE0-\u0CE3\u0CE6-\u0CEF\u0CF1\u0CF2\u0D01-\u0D03\u0D05-\u0D0C\u0D0E-\u0D10\u0D12-\u0D3A\u0D3D-\u0D44\u0D46-\u0D48\u0D4A-\u0D4E\u0D57\u0D5F-\u0D63\u0D66-\u0D6F\u0D7A-\u0D7F\u0D82\u0D83\u0D85-\u0D96\u0D9A-\u0DB1\u0DB3-\u0DBB\u0DBD\u0DC0-\u0DC6\u0DCA\u0DCF-\u0DD4\u0DD6\u0DD8-\u0DDF\u0DE6-\u0DEF\u0DF2\u0DF3\u0E01-\u0E3A\u0E40-\u0E4E\u0E50-\u0E59\u0E81\u0E82\u0E84\u0E87\u0E88\u0E8A\u0E8D\u0E94-\u0E97\u0E99-\u0E9F\u0EA1-\u0EA3\u0EA5\u0EA7\u0EAA\u0EAB\u0EAD-\u0EB9\u0EBB-\u0EBD\u0EC0-\u0EC4\u0EC6\u0EC8-\u0ECD\u0ED0-\u0ED9\u0EDC-\u0EDF\u0F00\u0F18\u0F19\u0F20-\u0F29\u0F35\u0F37\u0F39\u0F3E-\u0F47\u0F49-\u0F6C\u0F71-\u0F84\u0F86-\u0F97\u0F99-\u0FBC\u0FC6\u1000-\u1049\u1050-\u109D\u10A0-\u10C5\u10C7\u10CD\u10D0-\u10FA\u10FC-\u1248\u124A-\u124D\u1250-\u1256\u1258\u125A-\u125D\u1260-\u1288\u128A-\u128D\u1290-\u12B0\u12B2-\u12B5\u12B8-\u12BE\u12C0\u12C2-\u12C5\u12C8-\u12D6\u12D8-\u1310\u1312-\u1315\u1318-\u135A\u135D-\u135F\u1369-\u1371\u1380-\u138F\u13A0-\u13F5\u13F8-\u13FD\u1401-\u166C\u166F-\u167F\u1681-\u169A\u16A0-\u16EA\u16EE-\u16F8\u1700-\u170C\u170E-\u1714\u1720-\u1734\u1740-\u1753\u1760-\u176C\u176E-\u1770\u1772\u1773\u1780-\u17D3\u17D7\u17DC\u17DD\u17E0-\u17E9\u180B-\u180D\u1810-\u1819\u1820-\u1877\u1880-\u18AA\u18B0-\u18F5\u1900-\u191E\u1920-\u192B\u1930-\u193B\u1946-\u196D\u1970-\u1974\u1980-\u19AB\u19B0-\u19C9\u19D0-\u19DA\u1A00-\u1A1B\u1A20-\u1A5E\u1A60-\u1A7C\u1A7F-\u1A89\u1A90-\u1A99\u1AA7\u1AB0-\u1ABD\u1B00-\u1B4B\u1B50-\u1B59\u1B6B-\u1B73\u1B80-\u1BF3\u1C00-\u1C37\u1C40-\u1C49\u1C4D-\u1C7D\u1CD0-\u1CD2\u1CD4-\u1CF6\u1CF8\u1CF9\u1D00-\u1DF5\u1DFC-\u1F15\u1F18-\u1F1D\u1F20-\u1F45\u1F48-\u1F4D\u1F50-\u1F57\u1F59\u1F5B\u1F5D\u1F5F-\u1F7D\u1F80-\u1FB4\u1FB6-\u1FBC\u1FBE\u1FC2-\u1FC4\u1FC6-\u1FCC\u1FD0-\u1FD3\u1FD6-\u1FDB\u1FE0-\u1FEC\u1FF2-\u1FF4\u1FF6-\u1FFC\u200C\u200D\u203F\u2040\u2054\u2071\u207F\u2090-\u209C\u20D0-\u20DC\u20E1\u20E5-\u20F0\u2102\u2107\u210A-\u2113\u2115\u2118-\u211D\u2124\u2126\u2128\u212A-\u2139\u213C-\u213F\u2145-\u2149\u214E\u2160-\u2188\u2C00-\u2C2E\u2C30-\u2C5E\u2C60-\u2CE4\u2CEB-\u2CF3\u2D00-\u2D25\u2D27\u2D2D\u2D30-\u2D67\u2D6F\u2D7F-\u2D96\u2DA0-\u2DA6\u2DA8-\u2DAE\u2DB0-\u2DB6\u2DB8-\u2DBE\u2DC0-\u2DC6\u2DC8-\u2DCE\u2DD0-\u2DD6\u2DD8-\u2DDE\u2DE0-\u2DFF\u3005-\u3007\u3021-\u302F\u3031-\u3035\u3038-\u303C\u3041-\u3096\u3099-\u309F\u30A1-\u30FA\u30FC-\u30FF\u3105-\u312D\u3131-\u318E\u31A0-\u31BA\u31F0-\u31FF\u3400-\u4DB5\u4E00-\u9FD5\uA000-\uA48C\uA4D0-\uA4FD\uA500-\uA60C\uA610-\uA62B\uA640-\uA66F\uA674-\uA67D\uA67F-\uA6F1\uA717-\uA71F\uA722-\uA788\uA78B-\uA7AD\uA7B0-\uA7B7\uA7F7-\uA827\uA840-\uA873\uA880-\uA8C4\uA8D0-\uA8D9\uA8E0-\uA8F7\uA8FB\uA8FD\uA900-\uA92D\uA930-\uA953\uA960-\uA97C\uA980-\uA9C0\uA9CF-\uA9D9\uA9E0-\uA9FE\uAA00-\uAA36\uAA40-\uAA4D\uAA50-\uAA59\uAA60-\uAA76\uAA7A-\uAAC2\uAADB-\uAADD\uAAE0-\uAAEF\uAAF2-\uAAF6\uAB01-\uAB06\uAB09-\uAB0E\uAB11-\uAB16\uAB20-\uAB26\uAB28-\uAB2E\uAB30-\uAB5A\uAB5C-\uAB65\uAB70-\uABEA\uABEC\uABED\uABF0-\uABF9\uAC00-\uD7A3\uD7B0-\uD7C6\uD7CB-\uD7FB\uF900-\uFA6D\uFA70-\uFAD9\uFB00-\uFB06\uFB13-\uFB17\uFB1D-\uFB28\uFB2A-\uFB36\uFB38-\uFB3C\uFB3E\uFB40\uFB41\uFB43\uFB44\uFB46-\uFBB1\uFBD3-\uFD3D\uFD50-\uFD8F\uFD92-\uFDC7\uFDF0-\uFDFB\uFE00-\uFE0F\uFE20-\uFE2F\uFE33\uFE34\uFE4D-\uFE4F\uFE70-\uFE74\uFE76-\uFEFC\uFF10-\uFF19\uFF21-\uFF3A\uFF3F\uFF41-\uFF5A\uFF66-\uFFBE\uFFC2-\uFFC7\uFFCA-\uFFCF\uFFD2-\uFFD7\uFFDA-\uFFDC]|\uD800[\uDC00-\uDC0B\uDC0D-\uDC26\uDC28-\uDC3A\uDC3C\uDC3D\uDC3F-\uDC4D\uDC50-\uDC5D\uDC80-\uDCFA\uDD40-\uDD74\uDDFD\uDE80-\uDE9C\uDEA0-\uDED0\uDEE0\uDF00-\uDF1F\uDF30-\uDF4A\uDF50-\uDF7A\uDF80-\uDF9D\uDFA0-\uDFC3\uDFC8-\uDFCF\uDFD1-\uDFD5]|\uD801[\uDC00-\uDC9D\uDCA0-\uDCA9\uDD00-\uDD27\uDD30-\uDD63\uDE00-\uDF36\uDF40-\uDF55\uDF60-\uDF67]|\uD802[\uDC00-\uDC05\uDC08\uDC0A-\uDC35\uDC37\uDC38\uDC3C\uDC3F-\uDC55\uDC60-\uDC76\uDC80-\uDC9E\uDCE0-\uDCF2\uDCF4\uDCF5\uDD00-\uDD15\uDD20-\uDD39\uDD80-\uDDB7\uDDBE\uDDBF\uDE00-\uDE03\uDE05\uDE06\uDE0C-\uDE13\uDE15-\uDE17\uDE19-\uDE33\uDE38-\uDE3A\uDE3F\uDE60-\uDE7C\uDE80-\uDE9C\uDEC0-\uDEC7\uDEC9-\uDEE6\uDF00-\uDF35\uDF40-\uDF55\uDF60-\uDF72\uDF80-\uDF91]|\uD803[\uDC00-\uDC48\uDC80-\uDCB2\uDCC0-\uDCF2]|\uD804[\uDC00-\uDC46\uDC66-\uDC6F\uDC7F-\uDCBA\uDCD0-\uDCE8\uDCF0-\uDCF9\uDD00-\uDD34\uDD36-\uDD3F\uDD50-\uDD73\uDD76\uDD80-\uDDC4\uDDCA-\uDDCC\uDDD0-\uDDDA\uDDDC\uDE00-\uDE11\uDE13-\uDE37\uDE80-\uDE86\uDE88\uDE8A-\uDE8D\uDE8F-\uDE9D\uDE9F-\uDEA8\uDEB0-\uDEEA\uDEF0-\uDEF9\uDF00-\uDF03\uDF05-\uDF0C\uDF0F\uDF10\uDF13-\uDF28\uDF2A-\uDF30\uDF32\uDF33\uDF35-\uDF39\uDF3C-\uDF44\uDF47\uDF48\uDF4B-\uDF4D\uDF50\uDF57\uDF5D-\uDF63\uDF66-\uDF6C\uDF70-\uDF74]|\uD805[\uDC80-\uDCC5\uDCC7\uDCD0-\uDCD9\uDD80-\uDDB5\uDDB8-\uDDC0\uDDD8-\uDDDD\uDE00-\uDE40\uDE44\uDE50-\uDE59\uDE80-\uDEB7\uDEC0-\uDEC9\uDF00-\uDF19\uDF1D-\uDF2B\uDF30-\uDF39]|\uD806[\uDCA0-\uDCE9\uDCFF\uDEC0-\uDEF8]|\uD808[\uDC00-\uDF99]|\uD809[\uDC00-\uDC6E\uDC80-\uDD43]|[\uD80C\uD840-\uD868\uD86A-\uD86C\uD86F-\uD872][\uDC00-\uDFFF]|\uD80D[\uDC00-\uDC2E]|\uD811[\uDC00-\uDE46]|\uD81A[\uDC00-\uDE38\uDE40-\uDE5E\uDE60-\uDE69\uDED0-\uDEED\uDEF0-\uDEF4\uDF00-\uDF36\uDF40-\uDF43\uDF50-\uDF59\uDF63-\uDF77\uDF7D-\uDF8F]|\uD81B[\uDF00-\uDF44\uDF50-\uDF7E\uDF8F-\uDF9F]|\uD82C[\uDC00\uDC01]|\uD82F[\uDC00-\uDC6A\uDC70-\uDC7C\uDC80-\uDC88\uDC90-\uDC99\uDC9D\uDC9E]|\uD834[\uDD65-\uDD69\uDD6D-\uDD72\uDD7B-\uDD82\uDD85-\uDD8B\uDDAA-\uDDAD\uDE42-\uDE44]|\uD835[\uDC00-\uDC54\uDC56-\uDC9C\uDC9E\uDC9F\uDCA2\uDCA5\uDCA6\uDCA9-\uDCAC\uDCAE-\uDCB9\uDCBB\uDCBD-\uDCC3\uDCC5-\uDD05\uDD07-\uDD0A\uDD0D-\uDD14\uDD16-\uDD1C\uDD1E-\uDD39\uDD3B-\uDD3E\uDD40-\uDD44\uDD46\uDD4A-\uDD50\uDD52-\uDEA5\uDEA8-\uDEC0\uDEC2-\uDEDA\uDEDC-\uDEFA\uDEFC-\uDF14\uDF16-\uDF34\uDF36-\uDF4E\uDF50-\uDF6E\uDF70-\uDF88\uDF8A-\uDFA8\uDFAA-\uDFC2\uDFC4-\uDFCB\uDFCE-\uDFFF]|\uD836[\uDE00-\uDE36\uDE3B-\uDE6C\uDE75\uDE84\uDE9B-\uDE9F\uDEA1-\uDEAF]|\uD83A[\uDC00-\uDCC4\uDCD0-\uDCD6]|\uD83B[\uDE00-\uDE03\uDE05-\uDE1F\uDE21\uDE22\uDE24\uDE27\uDE29-\uDE32\uDE34-\uDE37\uDE39\uDE3B\uDE42\uDE47\uDE49\uDE4B\uDE4D-\uDE4F\uDE51\uDE52\uDE54\uDE57\uDE59\uDE5B\uDE5D\uDE5F\uDE61\uDE62\uDE64\uDE67-\uDE6A\uDE6C-\uDE72\uDE74-\uDE77\uDE79-\uDE7C\uDE7E\uDE80-\uDE89\uDE8B-\uDE9B\uDEA1-\uDEA3\uDEA5-\uDEA9\uDEAB-\uDEBB]|\uD869[\uDC00-\uDED6\uDF00-\uDFFF]|\uD86D[\uDC00-\uDF34\uDF40-\uDFFF]|\uD86E[\uDC00-\uDC1D\uDC20-\uDFFF]|\uD873[\uDC00-\uDEA1]|\uD87E[\uDC00-\uDE1D]|\uDB40[\uDD00-\uDDEF])+/,
};

try {
    UNICODE = {
        // https://262.ecma-international.org/13.0/#prod-IdentifierStartChar
        // $, _, ID_Start
        ID_Start: new RegExp("[_$\\p{ID_Start}]", "u"),
        // https://262.ecma-international.org/13.0/#prod-IdentifierPartChar
        // $, zero-width-joiner, zero-width-non-joiner, ID_Continue
        ID_Continue: new RegExp("[$\\u200C\\u200D\\p{ID_Continue}]+", "u"),
    };
} catch(e) {
    // Could not use modern JS \p{...}. UNICODE is already defined above so let's continue
}

function get_full_char(str, pos) {
    if (is_surrogate_pair_head(str.charCodeAt(pos))) {
        if (is_surrogate_pair_tail(str.charCodeAt(pos + 1))) {
            return str.charAt(pos) + str.charAt(pos + 1);
        }
    } else if (is_surrogate_pair_tail(str.charCodeAt(pos))) {
        if (is_surrogate_pair_head(str.charCodeAt(pos - 1))) {
            return str.charAt(pos - 1) + str.charAt(pos);
        }
    }
    return str.charAt(pos);
}

function get_full_char_code(str, pos) {
    // https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Surrogates
    if (is_surrogate_pair_head(str.charCodeAt(pos))) {
        return 0x10000 + (str.charCodeAt(pos) - 0xd800 << 10) + str.charCodeAt(pos + 1) - 0xdc00;
    }
    return str.charCodeAt(pos);
}

function get_full_char_length(str) {
    var surrogates = 0;

    for (var i = 0; i < str.length; i++) {
        if (is_surrogate_pair_head(str.charCodeAt(i)) && is_surrogate_pair_tail(str.charCodeAt(i + 1))) {
            surrogates++;
            i++;
        }
    }

    return str.length - surrogates;
}

function from_char_code(code) {
    // Based on https://github.com/mathiasbynens/String.fromCodePoint/blob/master/fromcodepoint.js
    if (code > 0xFFFF) {
        code -= 0x10000;
        return (String.fromCharCode((code >> 10) + 0xD800) +
            String.fromCharCode((code % 0x400) + 0xDC00));
    }
    return String.fromCharCode(code);
}

function is_surrogate_pair_head(code) {
    return code >= 0xd800 && code <= 0xdbff;
}

function is_surrogate_pair_tail(code) {
    return code >= 0xdc00 && code <= 0xdfff;
}

function is_digit(code) {
    return code >= 48 && code <= 57;
}

function is_identifier_start(ch) {
    return UNICODE.ID_Start.test(ch);
}

function is_identifier_char(ch) {
    return UNICODE.ID_Continue.test(ch);
}

const BASIC_IDENT = /^[a-z_$][a-z0-9_$]*$/i;

function is_basic_identifier_string(str) {
    return BASIC_IDENT.test(str);
}

function is_identifier_string(str, allow_surrogates) {
    if (BASIC_IDENT.test(str)) {
        return true;
    }
    if (!allow_surrogates && /[\ud800-\udfff]/.test(str)) {
        return false;
    }
    var match = UNICODE.ID_Start.exec(str);
    if (!match || match.index !== 0) {
        return false;
    }

    str = str.slice(match[0].length);
    if (!str) {
        return true;
    }

    match = UNICODE.ID_Continue.exec(str);
    return !!match && match[0].length === str.length;
}

function parse_js_number(num, allow_e = true) {
    if (!allow_e && num.includes("e")) {
        return NaN;
    }
    if (RE_HEX_NUMBER.test(num)) {
        return parseInt(num.substr(2), 16);
    } else if (RE_OCT_NUMBER.test(num)) {
        return parseInt(num.substr(1), 8);
    } else if (RE_ES6_OCT_NUMBER.test(num)) {
        return parseInt(num.substr(2), 8);
    } else if (RE_BIN_NUMBER.test(num)) {
        return parseInt(num.substr(2), 2);
    } else if (RE_DEC_NUMBER.test(num)) {
        return parseFloat(num);
    } else {
        var val = parseFloat(num);
        if (val == num) return val;
    }
}

class JS_Parse_Error extends Error {
    constructor(message, filename, line, col, pos) {
        super();

        this.name = "SyntaxError";
        this.message = message;
        this.filename = filename;
        this.line = line;
        this.col = col;
        this.pos = pos;
    }
}

function js_error(message, filename, line, col, pos) {
    throw new JS_Parse_Error(message, filename, line, col, pos);
}

function is_token(token, type, val) {
    return token.type == type && (val == null || token.value == val);
}

var EX_EOF = {};

function tokenizer($TEXT, filename, html5_comments, shebang) {
    var S = {
        text            : $TEXT,
        filename        : filename,
        pos             : 0,
        tokpos          : 0,
        line            : 1,
        tokline         : 0,
        col             : 0,
        tokcol          : 0,
        newline_before  : false,
        regex_allowed   : false,
        brace_counter   : 0,
        template_braces : [],
        comments_before : [],
        directives      : {},
        directive_stack : []
    };

    function peek() { return get_full_char(S.text, S.pos); }

    // Used because parsing ?. involves a lookahead for a digit
    function is_option_chain_op() {
        const must_be_dot = S.text.charCodeAt(S.pos + 1) === 46;
        if (!must_be_dot) return false;

        const cannot_be_digit = S.text.charCodeAt(S.pos + 2);
        return cannot_be_digit < 48 || cannot_be_digit > 57;
    }

    function next(signal_eof, in_string) {
        var ch = get_full_char(S.text, S.pos++);
        if (signal_eof && !ch)
            throw EX_EOF;
        if (NEWLINE_CHARS.has(ch)) {
            S.newline_before = S.newline_before || !in_string;
            ++S.line;
            S.col = 0;
            if (ch == "\r" && peek() == "\n") {
                // treat a \r\n sequence as a single \n
                ++S.pos;
                ch = "\n";
            }
        } else {
            if (ch.length > 1) {
                ++S.pos;
                ++S.col;
            }
            ++S.col;
        }
        return ch;
    }

    function forward(i) {
        while (i--) next();
    }

    function looking_at(str) {
        return S.text.substr(S.pos, str.length) == str;
    }

    function find_eol() {
        var text = S.text;
        for (var i = S.pos, n = S.text.length; i < n; ++i) {
            var ch = text[i];
            if (NEWLINE_CHARS.has(ch))
                return i;
        }
        return -1;
    }

    function find(what, signal_eof) {
        var pos = S.text.indexOf(what, S.pos);
        if (signal_eof && pos == -1) throw EX_EOF;
        return pos;
    }

    function start_token() {
        S.tokline = S.line;
        S.tokcol = S.col;
        S.tokpos = S.pos;
    }

    var prev_was_dot = false;
    var previous_token = null;
    function token(type, value, is_comment) {
        S.regex_allowed = ((type == "operator" && !UNARY_POSTFIX.has(value)) ||
                           (type == "keyword" && KEYWORDS_BEFORE_EXPRESSION.has(value)) ||
                           (type == "punc" && PUNC_BEFORE_EXPRESSION.has(value))) ||
                           (type == "arrow");
        if (type == "punc" && (value == "." || value == "?.")) {
            prev_was_dot = true;
        } else if (!is_comment) {
            prev_was_dot = false;
        }
        const line     = S.tokline;
        const col      = S.tokcol;
        const pos      = S.tokpos;
        const nlb      = S.newline_before;
        const file     = filename;
        let comments_before = [];
        let comments_after  = [];

        if (!is_comment) {
            comments_before = S.comments_before;
            comments_after = S.comments_before = [];
        }
        S.newline_before = false;
        const tok = new AST_Token(type, value, line, col, pos, nlb, comments_before, comments_after, file);

        if (!is_comment) previous_token = tok;
        return tok;
    }

    function skip_whitespace() {
        while (WHITESPACE_CHARS.has(peek()))
            next();
    }

    function read_while(pred) {
        var ret = "", ch, i = 0;
        while ((ch = peek()) && pred(ch, i++))
            ret += next();
        return ret;
    }

    function parse_error(err) {
        js_error(err, filename, S.tokline, S.tokcol, S.tokpos);
    }

    function read_num(prefix) {
        var has_e = false, after_e = false, has_x = false, has_dot = prefix == ".", is_big_int = false, numeric_separator = false;
        var num = read_while(function(ch, i) {
            if (is_big_int) return false;

            var code = ch.charCodeAt(0);
            switch (code) {
              case 95: // _
                return (numeric_separator = true);
              case 98: case 66: // bB
                return (has_x = true); // Can occur in hex sequence, don't return false yet
              case 111: case 79: // oO
              case 120: case 88: // xX
                return has_x ? false : (has_x = true);
              case 101: case 69: // eE
                return has_x ? true : has_e ? false : (has_e = after_e = true);
              case 45: // -
                return after_e || (i == 0 && !prefix);
              case 43: // +
                return after_e;
              case (after_e = false, 46): // .
                return (!has_dot && !has_x && !has_e) ? (has_dot = true) : false;
            }

            if (ch === "n") {
                is_big_int = true;

                return true;
            }

            return RE_NUM_LITERAL.test(ch);
        });
        if (prefix) num = prefix + num;

        LATEST_RAW = num;

        if (RE_OCT_NUMBER.test(num) && next_token.has_directive("use strict")) {
            parse_error("Legacy octal literals are not allowed in strict mode");
        }
        if (numeric_separator) {
            if (num.endsWith("_")) {
                parse_error("Numeric separators are not allowed at the end of numeric literals");
            } else if (num.includes("__")) {
                parse_error("Only one underscore is allowed as numeric separator");
            }
            num = num.replace(/_/g, "");
        }
        if (num.endsWith("n")) {
            const without_n = num.slice(0, -1);
            const allow_e = RE_HEX_NUMBER.test(without_n);
            const valid = parse_js_number(without_n, allow_e);
            if (!has_dot && RE_BIG_INT.test(num) && !isNaN(valid))
                return token("big_int", without_n);
            parse_error("Invalid or unexpected token");
        }
        var valid = parse_js_number(num);
        if (!isNaN(valid)) {
            return token("num", valid);
        } else {
            parse_error("Invalid syntax: " + num);
        }
    }

    function is_octal(ch) {
        return ch >= "0" && ch <= "7";
    }

    function read_escaped_char(in_string, strict_hex, template_string) {
        var ch = next(true, in_string);
        switch (ch.charCodeAt(0)) {
          case 110 : return "\n";
          case 114 : return "\r";
          case 116 : return "\t";
          case 98  : return "\b";
          case 118 : return "\u000b"; // \v
          case 102 : return "\f";
          case 120 : return String.fromCharCode(hex_bytes(2, strict_hex)); // \x
          case 117 : // \u
            if (peek() == "{") {
                next(true);
                if (peek() === "}")
                    parse_error("Expecting hex-character between {}");
                while (peek() == "0") next(true); // No significance
                var result, length = find("}", true) - S.pos;
                // Avoid 32 bit integer overflow (1 << 32 === 1)
                // We know first character isn't 0 and thus out of range anyway
                if (length > 6 || (result = hex_bytes(length, strict_hex)) > 0x10FFFF) {
                    parse_error("Unicode reference out of bounds");
                }
                next(true);
                return from_char_code(result);
            }
            return String.fromCharCode(hex_bytes(4, strict_hex));
          case 10  : return ""; // newline
          case 13  :            // \r
            if (peek() == "\n") { // DOS newline
                next(true, in_string);
                return "";
            }
        }
        if (is_octal(ch)) {
            if (template_string && strict_hex) {
                const represents_null_character = ch === "0" && !is_octal(peek());
                if (!represents_null_character) {
                    parse_error("Octal escape sequences are not allowed in template strings");
                }
            }
            return read_octal_escape_sequence(ch, strict_hex);
        }
        return ch;
    }

    function read_octal_escape_sequence(ch, strict_octal) {
        // Read
        var p = peek();
        if (p >= "0" && p <= "7") {
            ch += next(true);
            if (ch[0] <= "3" && (p = peek()) >= "0" && p <= "7")
                ch += next(true);
        }

        // Parse
        if (ch === "0") return "\0";
        if (ch.length > 0 && next_token.has_directive("use strict") && strict_octal)
            parse_error("Legacy octal escape sequences are not allowed in strict mode");
        return String.fromCharCode(parseInt(ch, 8));
    }

    function hex_bytes(n, strict_hex) {
        var num = 0;
        for (; n > 0; --n) {
            if (!strict_hex && isNaN(parseInt(peek(), 16))) {
                return parseInt(num, 16) || "";
            }
            var digit = next(true);
            if (isNaN(parseInt(digit, 16)))
                parse_error("Invalid hex-character pattern in string");
            num += digit;
        }
        return parseInt(num, 16);
    }

    var read_string = with_eof_error("Unterminated string constant", function() {
        const start_pos = S.pos;
        var quote = next(), ret = [];
        for (;;) {
            var ch = next(true, true);
            if (ch == "\\") ch = read_escaped_char(true, true);
            else if (ch == "\r" || ch == "\n") parse_error("Unterminated string constant");
            else if (ch == quote) break;
            ret.push(ch);
        }
        var tok = token("string", ret.join(""));
        LATEST_RAW = S.text.slice(start_pos, S.pos);
        tok.quote = quote;
        return tok;
    });

    var read_template_characters = with_eof_error("Unterminated template", function(begin) {
        if (begin) {
            S.template_braces.push(S.brace_counter);
        }
        var content = "", raw = "", ch, tok;
        next(true, true);
        while ((ch = next(true, true)) != "`") {
            if (ch == "\r") {
                if (peek() == "\n") ++S.pos;
                ch = "\n";
            } else if (ch == "$" && peek() == "{") {
                next(true, true);
                S.brace_counter++;
                tok = token(begin ? "template_head" : "template_substitution", content);
                TEMPLATE_RAWS.set(tok, raw);
                tok.template_end = false;
                return tok;
            }

            raw += ch;
            if (ch == "\\") {
                var tmp = S.pos;
                var prev_is_tag = previous_token && (previous_token.type === "name" || previous_token.type === "punc" && (previous_token.value === ")" || previous_token.value === "]"));
                ch = read_escaped_char(true, !prev_is_tag, true);
                raw += S.text.substr(tmp, S.pos - tmp);
            }

            content += ch;
        }
        S.template_braces.pop();
        tok = token(begin ? "template_head" : "template_substitution", content);
        TEMPLATE_RAWS.set(tok, raw);
        tok.template_end = true;
        return tok;
    });

    function skip_line_comment(type) {
        var regex_allowed = S.regex_allowed;
        var i = find_eol(), ret;
        if (i == -1) {
            ret = S.text.substr(S.pos);
            S.pos = S.text.length;
        } else {
            ret = S.text.substring(S.pos, i);
            S.pos = i;
        }
        S.col = S.tokcol + (S.pos - S.tokpos);
        S.comments_before.push(token(type, ret, true));
        S.regex_allowed = regex_allowed;
        return next_token;
    }

    var skip_multiline_comment = with_eof_error("Unterminated multiline comment", function() {
        var regex_allowed = S.regex_allowed;
        var i = find("*/", true);
        var text = S.text.substring(S.pos, i).replace(/\r\n|\r|\u2028|\u2029/g, "\n");
        // update stream position
        forward(get_full_char_length(text) /* text length doesn't count \r\n as 2 char while S.pos - i does */ + 2);
        S.comments_before.push(token("comment2", text, true));
        S.newline_before = S.newline_before || text.includes("\n");
        S.regex_allowed = regex_allowed;
        return next_token;
    });

    var read_name = with_eof_error("Unterminated identifier name", function() {
        var name = [], ch, escaped = false;
        var read_escaped_identifier_char = function() {
            escaped = true;
            next();
            if (peek() !== "u") {
                parse_error("Expecting UnicodeEscapeSequence -- uXXXX or u{XXXX}");
            }
            return read_escaped_char(false, true);
        };

        // Read first character (ID_Start)
        if ((ch = peek()) === "\\") {
            ch = read_escaped_identifier_char();
            if (!is_identifier_start(ch)) {
                parse_error("First identifier char is an invalid identifier char");
            }
        } else if (is_identifier_start(ch)) {
            next();
        } else {
            return "";
        }

        name.push(ch);

        // Read ID_Continue
        while ((ch = peek()) != null) {
            if ((ch = peek()) === "\\") {
                ch = read_escaped_identifier_char();
                if (!is_identifier_char(ch)) {
                    parse_error("Invalid escaped identifier char");
                }
            } else {
                if (!is_identifier_char(ch)) {
                    break;
                }
                next();
            }
            name.push(ch);
        }
        const name_str = name.join("");
        if (RESERVED_WORDS.has(name_str) && escaped) {
            parse_error("Escaped characters are not allowed in keywords");
        }
        return name_str;
    });

    var read_regexp = with_eof_error("Unterminated regular expression", function(source) {
        var prev_backslash = false, ch, in_class = false;
        while ((ch = next(true))) if (NEWLINE_CHARS.has(ch)) {
            parse_error("Unexpected line terminator");
        } else if (prev_backslash) {
            source += "\\" + ch;
            prev_backslash = false;
        } else if (ch == "[") {
            in_class = true;
            source += ch;
        } else if (ch == "]" && in_class) {
            in_class = false;
            source += ch;
        } else if (ch == "/" && !in_class) {
            break;
        } else if (ch == "\\") {
            prev_backslash = true;
        } else {
            source += ch;
        }
        const flags = read_name();
        return token("regexp", "/" + source + "/" + flags);
    });

    function read_operator(prefix) {
        function grow(op) {
            if (!peek()) return op;
            var bigger = op + peek();
            if (OPERATORS.has(bigger)) {
                next();
                return grow(bigger);
            } else {
                return op;
            }
        }
        return token("operator", grow(prefix || next()));
    }

    function handle_slash() {
        next();
        switch (peek()) {
          case "/":
            next();
            return skip_line_comment("comment1");
          case "*":
            next();
            return skip_multiline_comment();
        }
        return S.regex_allowed ? read_regexp("") : read_operator("/");
    }

    function handle_eq_sign() {
        next();
        if (peek() === ">") {
            next();
            return token("arrow", "=>");
        } else {
            return read_operator("=");
        }
    }

    function handle_dot() {
        next();
        if (is_digit(peek().charCodeAt(0))) {
            return read_num(".");
        }
        if (peek() === ".") {
            next();  // Consume second dot
            next();  // Consume third dot
            return token("expand", "...");
        }

        return token("punc", ".");
    }

    function read_word() {
        var word = read_name();
        if (prev_was_dot) return token("name", word);
        return KEYWORDS_ATOM.has(word) ? token("atom", word)
            : !KEYWORDS.has(word) ? token("name", word)
            : OPERATORS.has(word) ? token("operator", word)
            : token("keyword", word);
    }

    function read_private_word() {
        next();
        return token("privatename", read_name());
    }

    function with_eof_error(eof_error, cont) {
        return function(x) {
            try {
                return cont(x);
            } catch(ex) {
                if (ex === EX_EOF) parse_error(eof_error);
                else throw ex;
            }
        };
    }

    function next_token(force_regexp) {
        if (force_regexp != null)
            return read_regexp(force_regexp);
        if (shebang && S.pos == 0 && looking_at("#!")) {
            start_token();
            forward(2);
            skip_line_comment("comment5");
        }
        for (;;) {
            skip_whitespace();
            start_token();
            if (html5_comments) {
                if (looking_at("<!--")) {
                    forward(4);
                    skip_line_comment("comment3");
                    continue;
                }
                if (looking_at("-->") && S.newline_before) {
                    forward(3);
                    skip_line_comment("comment4");
                    continue;
                }
            }
            var ch = peek();
            if (!ch) return token("eof");
            var code = ch.charCodeAt(0);
            switch (code) {
              case 34: case 39: return read_string();
              case 46: return handle_dot();
              case 47: {
                  var tok = handle_slash();
                  if (tok === next_token) continue;
                  return tok;
              }
              case 61: return handle_eq_sign();
              case 63: {
                  if (!is_option_chain_op()) break;  // Handled below

                  next(); // ?
                  next(); // .

                  return token("punc", "?.");
              }
              case 96: return read_template_characters(true);
              case 123:
                S.brace_counter++;
                break;
              case 125:
                S.brace_counter--;
                if (S.template_braces.length > 0
                    && S.template_braces[S.template_braces.length - 1] === S.brace_counter)
                    return read_template_characters(false);
                break;
            }
            if (is_digit(code)) return read_num();
            if (PUNC_CHARS.has(ch)) return token("punc", next());
            if (OPERATOR_CHARS.has(ch)) return read_operator();
            if (code == 92 || is_identifier_start(ch)) return read_word();
            if (code == 35) return read_private_word();
            break;
        }
        parse_error("Unexpected character '" + ch + "'");
    }

    next_token.next = next;
    next_token.peek = peek;

    next_token.context = function(nc) {
        if (nc) S = nc;
        return S;
    };

    next_token.add_directive = function(directive) {
        S.directive_stack[S.directive_stack.length - 1].push(directive);

        if (S.directives[directive] === undefined) {
            S.directives[directive] = 1;
        } else {
            S.directives[directive]++;
        }
    };

    next_token.push_directives_stack = function() {
        S.directive_stack.push([]);
    };

    next_token.pop_directives_stack = function() {
        var directives = S.directive_stack[S.directive_stack.length - 1];

        for (var i = 0; i < directives.length; i++) {
            S.directives[directives[i]]--;
        }

        S.directive_stack.pop();
    };

    next_token.has_directive = function(directive) {
        return S.directives[directive] > 0;
    };

    return next_token;

}

/* -----[ Parser (constants) ]----- */

var UNARY_PREFIX = makePredicate([
    "typeof",
    "void",
    "delete",
    "--",
    "++",
    "!",
    "~",
    "-",
    "+"
]);

var UNARY_POSTFIX = makePredicate([ "--", "++" ]);

var ASSIGNMENT = makePredicate([ "=", "+=", "-=", "??=", "&&=", "||=", "/=", "*=", "**=", "%=", ">>=", "<<=", ">>>=", "|=", "^=", "&=" ]);

var LOGICAL_ASSIGNMENT = makePredicate([ "??=", "&&=", "||=" ]);

var PRECEDENCE = (function(a, ret) {
    for (var i = 0; i < a.length; ++i) {
        var b = a[i];
        for (var j = 0; j < b.length; ++j) {
            ret[b[j]] = i + 1;
        }
    }
    return ret;
})(
    [
        ["||"],
        ["??"],
        ["&&"],
        ["|"],
        ["^"],
        ["&"],
        ["==", "===", "!=", "!=="],
        ["<", ">", "<=", ">=", "in", "instanceof"],
        [">>", "<<", ">>>"],
        ["+", "-"],
        ["*", "/", "%"],
        ["**"]
    ],
    {}
);

var ATOMIC_START_TOKEN = makePredicate([ "atom", "num", "big_int", "string", "regexp", "name"]);

/* -----[ Parser ]----- */

function parse($TEXT, options) {
    // maps start tokens to count of comments found outside of their parens
    // Example: /* I count */ ( /* I don't */ foo() )
    // Useful because comments_before property of call with parens outside
    // contains both comments inside and outside these parens. Used to find the
    // right #__PURE__ comments for an expression
    const outer_comments_before_counts = new WeakMap();

    options = defaults(options, {
        bare_returns   : false,
        ecma           : null,  // Legacy
        expression     : false,
        filename       : null,
        html5_comments : true,
        module         : false,
        shebang        : true,
        strict         : false,
        toplevel       : null,
    }, true);

    var S = {
        input         : (typeof $TEXT == "string"
                         ? tokenizer($TEXT, options.filename,
                                     options.html5_comments, options.shebang)
                         : $TEXT),
        token         : null,
        prev          : null,
        peeked        : null,
        in_function   : 0,
        in_async      : -1,
        in_generator  : -1,
        in_directives : true,
        in_loop       : 0,
        labels        : []
    };

    S.token = next();

    function is(type, value) {
        return is_token(S.token, type, value);
    }

    function peek() { return S.peeked || (S.peeked = S.input()); }

    function next() {
        S.prev = S.token;

        if (!S.peeked) peek();
        S.token = S.peeked;
        S.peeked = null;
        S.in_directives = S.in_directives && (
            S.token.type == "string" || is("punc", ";")
        );
        return S.token;
    }

    function prev() {
        return S.prev;
    }

    function croak(msg, line, col, pos) {
        var ctx = S.input.context();
        js_error(msg,
                 ctx.filename,
                 line != null ? line : ctx.tokline,
                 col != null ? col : ctx.tokcol,
                 pos != null ? pos : ctx.tokpos);
    }

    function token_error(token, msg) {
        croak(msg, token.line, token.col);
    }

    function unexpected(token) {
        if (token == null)
            token = S.token;
        token_error(token, "Unexpected token: " + token.type + " (" + token.value + ")");
    }

    function expect_token(type, val) {
        if (is(type, val)) {
            return next();
        }
        token_error(S.token, "Unexpected token " + S.token.type + " «" + S.token.value + "»" + ", expected " + type + " «" + val + "»");
    }

    function expect(punc) { return expect_token("punc", punc); }

    function has_newline_before(token) {
        return token.nlb || !token.comments_before.every((comment) => !comment.nlb);
    }

    function can_insert_semicolon() {
        return !options.strict
            && (is("eof") || is("punc", "}") || has_newline_before(S.token));
    }

    function is_in_generator() {
        return S.in_generator === S.in_function;
    }

    function is_in_async() {
        return S.in_async === S.in_function;
    }

    function can_await() {
        return (
            S.in_async === S.in_function
            || S.in_function === 0 && S.input.has_directive("use strict")
        );
    }

    function semicolon(optional) {
        if (is("punc", ";")) next();
        else if (!optional && !can_insert_semicolon()) unexpected();
    }

    function parenthesised() {
        expect("(");
        var exp = expression(true);
        expect(")");
        return exp;
    }

    function embed_tokens(parser) {
        return function _embed_tokens_wrapper(...args) {
            const start = S.token;
            const expr = parser(...args);
            expr.start = start;
            expr.end = prev();
            return expr;
        };
    }

    function handle_regexp() {
        if (is("operator", "/") || is("operator", "/=")) {
            S.peeked = null;
            S.token = S.input(S.token.value.substr(1)); // force regexp
        }
    }

    var statement = embed_tokens(function statement(is_export_default, is_for_body, is_if_body) {
        handle_regexp();
        switch (S.token.type) {
          case "string":
            if (S.in_directives) {
                var token = peek();
                if (!LATEST_RAW.includes("\\")
                    && (is_token(token, "punc", ";")
                        || is_token(token, "punc", "}")
                        || has_newline_before(token)
                        || is_token(token, "eof"))) {
                    S.input.add_directive(S.token.value);
                } else {
                    S.in_directives = false;
                }
            }
            var dir = S.in_directives, stat = simple_statement();
            return dir && stat.body instanceof AST_String ? new AST_Directive(stat.body) : stat;
          case "template_head":
          case "num":
          case "big_int":
          case "regexp":
          case "operator":
          case "atom":
            return simple_statement();

          case "name":
          case "privatename":
            if(is("privatename") && !S.in_class)
                croak("Private field must be used in an enclosing class");

            if (S.token.value == "async" && is_token(peek(), "keyword", "function")) {
                next();
                next();
                if (is_for_body) {
                    croak("functions are not allowed as the body of a loop");
                }
                return function_(AST_Defun, false, true, is_export_default);
            }
            if (S.token.value == "import" && !is_token(peek(), "punc", "(") && !is_token(peek(), "punc", ".")) {
                next();
                var node = import_statement();
                semicolon();
                return node;
            }
            return is_token(peek(), "punc", ":")
                ? labeled_statement()
                : simple_statement();

          case "punc":
            switch (S.token.value) {
              case "{":
                return new AST_BlockStatement({
                    start : S.token,
                    body  : block_(),
                    end   : prev()
                });
              case "[":
              case "(":
                return simple_statement();
              case ";":
                S.in_directives = false;
                next();
                return new AST_EmptyStatement();
              default:
                unexpected();
            }

          case "keyword":
            switch (S.token.value) {
              case "break":
                next();
                return break_cont(AST_Break);

              case "continue":
                next();
                return break_cont(AST_Continue);

              case "debugger":
                next();
                semicolon();
                return new AST_Debugger();

              case "do":
                next();
                var body = in_loop(statement);
                expect_token("keyword", "while");
                var condition = parenthesised();
                semicolon(true);
                return new AST_Do({
                    body      : body,
                    condition : condition
                });

              case "while":
                next();
                return new AST_While({
                    condition : parenthesised(),
                    body      : in_loop(function() { return statement(false, true); })
                });

              case "for":
                next();
                return for_();

              case "class":
                next();
                if (is_for_body) {
                    croak("classes are not allowed as the body of a loop");
                }
                if (is_if_body) {
                    croak("classes are not allowed as the body of an if");
                }
                return class_(AST_DefClass, is_export_default);

              case "function":
                next();
                if (is_for_body) {
                    croak("functions are not allowed as the body of a loop");
                }
                return function_(AST_Defun, false, false, is_export_default);

              case "if":
                next();
                return if_();

              case "return":
                if (S.in_function == 0 && !options.bare_returns)
                    croak("'return' outside of function");
                next();
                var value = null;
                if (is("punc", ";")) {
                    next();
                } else if (!can_insert_semicolon()) {
                    value = expression(true);
                    semicolon();
                }
                return new AST_Return({
                    value: value
                });

              case "switch":
                next();
                return new AST_Switch({
                    expression : parenthesised(),
                    body       : in_loop(switch_body_)
                });

              case "throw":
                next();
                if (has_newline_before(S.token))
                    croak("Illegal newline after 'throw'");
                var value = expression(true);
                semicolon();
                return new AST_Throw({
                    value: value
                });

              case "try":
                next();
                return try_();

              case "var":
                next();
                var node = var_();
                semicolon();
                return node;

              case "let":
                next();
                var node = let_();
                semicolon();
                return node;

              case "const":
                next();
                var node = const_();
                semicolon();
                return node;

              case "with":
                if (S.input.has_directive("use strict")) {
                    croak("Strict mode may not include a with statement");
                }
                next();
                return new AST_With({
                    expression : parenthesised(),
                    body       : statement()
                });

              case "export":
                if (!is_token(peek(), "punc", "(")) {
                    next();
                    var node = export_statement();
                    if (is("punc", ";")) semicolon();
                    return node;
                }
            }
        }
        unexpected();
    });

    function labeled_statement() {
        var label = as_symbol(AST_Label);
        if (label.name === "await" && is_in_async()) {
            token_error(S.prev, "await cannot be used as label inside async function");
        }
        if (S.labels.some((l) => l.name === label.name)) {
            // ECMA-262, 12.12: An ECMAScript program is considered
            // syntactically incorrect if it contains a
            // LabelledStatement that is enclosed by a
            // LabelledStatement with the same Identifier as label.
            croak("Label " + label.name + " defined twice");
        }
        expect(":");
        S.labels.push(label);
        var stat = statement();
        S.labels.pop();
        if (!(stat instanceof AST_IterationStatement)) {
            // check for `continue` that refers to this label.
            // those should be reported as syntax errors.
            // https://github.com/mishoo/UglifyJS2/issues/287
            label.references.forEach(function(ref) {
                if (ref instanceof AST_Continue) {
                    ref = ref.label.start;
                    croak("Continue label `" + label.name + "` refers to non-IterationStatement.",
                          ref.line, ref.col, ref.pos);
                }
            });
        }
        return new AST_LabeledStatement({ body: stat, label: label });
    }

    function simple_statement(tmp) {
        return new AST_SimpleStatement({ body: (tmp = expression(true), semicolon(), tmp) });
    }

    function break_cont(type) {
        var label = null, ldef;
        if (!can_insert_semicolon()) {
            label = as_symbol(AST_LabelRef, true);
        }
        if (label != null) {
            ldef = S.labels.find((l) => l.name === label.name);
            if (!ldef)
                croak("Undefined label " + label.name);
            label.thedef = ldef;
        } else if (S.in_loop == 0)
            croak(type.TYPE + " not inside a loop or switch");
        semicolon();
        var stat = new type({ label: label });
        if (ldef) ldef.references.push(stat);
        return stat;
    }

    function for_() {
        var for_await_error = "`for await` invalid in this context";
        var await_tok = S.token;
        if (await_tok.type == "name" && await_tok.value == "await") {
            if (!can_await()) {
                token_error(await_tok, for_await_error);
            }
            next();
        } else {
            await_tok = false;
        }
        expect("(");
        var init = null;
        if (!is("punc", ";")) {
            init =
                is("keyword", "var") ? (next(), var_(true)) :
                is("keyword", "let") ? (next(), let_(true)) :
                is("keyword", "const") ? (next(), const_(true)) :
                                       expression(true, true);
            var is_in = is("operator", "in");
            var is_of = is("name", "of");
            if (await_tok && !is_of) {
                token_error(await_tok, for_await_error);
            }
            if (is_in || is_of) {
                if (init instanceof AST_Definitions) {
                    if (init.definitions.length > 1)
                        token_error(init.start, "Only one variable declaration allowed in for..in loop");
                } else if (!(is_assignable(init) || (init = to_destructuring(init)) instanceof AST_Destructuring)) {
                    token_error(init.start, "Invalid left-hand side in for..in loop");
                }
                next();
                if (is_in) {
                    return for_in(init);
                } else {
                    return for_of(init, !!await_tok);
                }
            }
        } else if (await_tok) {
            token_error(await_tok, for_await_error);
        }
        return regular_for(init);
    }

    function regular_for(init) {
        expect(";");
        var test = is("punc", ";") ? null : expression(true);
        expect(";");
        var step = is("punc", ")") ? null : expression(true);
        expect(")");
        return new AST_For({
            init      : init,
            condition : test,
            step      : step,
            body      : in_loop(function() { return statement(false, true); })
        });
    }

    function for_of(init, is_await) {
        var lhs = init instanceof AST_Definitions ? init.definitions[0].name : null;
        var obj = expression(true);
        expect(")");
        return new AST_ForOf({
            await  : is_await,
            init   : init,
            name   : lhs,
            object : obj,
            body   : in_loop(function() { return statement(false, true); })
        });
    }

    function for_in(init) {
        var obj = expression(true);
        expect(")");
        return new AST_ForIn({
            init   : init,
            object : obj,
            body   : in_loop(function() { return statement(false, true); })
        });
    }

    var arrow_function = function(start, argnames, is_async) {
        if (has_newline_before(S.token)) {
            croak("Unexpected newline before arrow (=>)");
        }

        expect_token("arrow", "=>");

        var body = _function_body(is("punc", "{"), false, is_async);

        var end =
            body instanceof Array && body.length ? body[body.length - 1].end :
            body instanceof Array ? start :
                body.end;

        return new AST_Arrow({
            start    : start,
            end      : end,
            async    : is_async,
            argnames : argnames,
            body     : body
        });
    };

    var function_ = function(ctor, is_generator_property, is_async, is_export_default) {
        var in_statement = ctor === AST_Defun;
        var is_generator = is("operator", "*");
        if (is_generator) {
            next();
        }

        var name = is("name") ? as_symbol(in_statement ? AST_SymbolDefun : AST_SymbolLambda) : null;
        if (in_statement && !name) {
            if (is_export_default) {
                ctor = AST_Function;
            } else {
                unexpected();
            }
        }

        if (name && ctor !== AST_Accessor && !(name instanceof AST_SymbolDeclaration))
            unexpected(prev());

        var args = [];
        var body = _function_body(true, is_generator || is_generator_property, is_async, name, args);
        return new ctor({
            start : args.start,
            end   : body.end,
            is_generator: is_generator,
            async : is_async,
            name  : name,
            argnames: args,
            body  : body
        });
    };

    class UsedParametersTracker {
        constructor(is_parameter, strict, duplicates_ok = false) {
            this.is_parameter = is_parameter;
            this.duplicates_ok = duplicates_ok;
            this.parameters = new Set();
            this.duplicate = null;
            this.default_assignment = false;
            this.spread = false;
            this.strict_mode = !!strict;
        }
        add_parameter(token) {
            if (this.parameters.has(token.value)) {
                if (this.duplicate === null) {
                    this.duplicate = token;
                }
                this.check_strict();
            } else {
                this.parameters.add(token.value);
                if (this.is_parameter) {
                    switch (token.value) {
                      case "arguments":
                      case "eval":
                      case "yield":
                        if (this.strict_mode) {
                            token_error(token, "Unexpected " + token.value + " identifier as parameter inside strict mode");
                        }
                        break;
                      default:
                        if (RESERVED_WORDS.has(token.value)) {
                            unexpected();
                        }
                    }
                }
            }
        }
        mark_default_assignment(token) {
            if (this.default_assignment === false) {
                this.default_assignment = token;
            }
        }
        mark_spread(token) {
            if (this.spread === false) {
                this.spread = token;
            }
        }
        mark_strict_mode() {
            this.strict_mode = true;
        }
        is_strict() {
            return this.default_assignment !== false || this.spread !== false || this.strict_mode;
        }
        check_strict() {
            if (this.is_strict() && this.duplicate !== null && !this.duplicates_ok) {
                token_error(this.duplicate, "Parameter " + this.duplicate.value + " was used already");
            }
        }
    }

    function parameters(params) {
        var used_parameters = new UsedParametersTracker(true, S.input.has_directive("use strict"));

        expect("(");

        while (!is("punc", ")")) {
            var param = parameter(used_parameters);
            params.push(param);

            if (!is("punc", ")")) {
                expect(",");
            }

            if (param instanceof AST_Expansion) {
                break;
            }
        }

        next();
    }

    function parameter(used_parameters, symbol_type) {
        var param;
        var expand = false;
        if (used_parameters === undefined) {
            used_parameters = new UsedParametersTracker(true, S.input.has_directive("use strict"));
        }
        if (is("expand", "...")) {
            expand = S.token;
            used_parameters.mark_spread(S.token);
            next();
        }
        param = binding_element(used_parameters, symbol_type);

        if (is("operator", "=") && expand === false) {
            used_parameters.mark_default_assignment(S.token);
            next();
            param = new AST_DefaultAssign({
                start: param.start,
                left: param,
                operator: "=",
                right: expression(false),
                end: S.token
            });
        }

        if (expand !== false) {
            if (!is("punc", ")")) {
                unexpected();
            }
            param = new AST_Expansion({
                start: expand,
                expression: param,
                end: expand
            });
        }
        used_parameters.check_strict();

        return param;
    }

    function binding_element(used_parameters, symbol_type) {
        var elements = [];
        var first = true;
        var is_expand = false;
        var expand_token;
        var first_token = S.token;
        if (used_parameters === undefined) {
            const strict = S.input.has_directive("use strict");
            const duplicates_ok = symbol_type === AST_SymbolVar;
            used_parameters = new UsedParametersTracker(false, strict, duplicates_ok);
        }
        symbol_type = symbol_type === undefined ? AST_SymbolFunarg : symbol_type;
        if (is("punc", "[")) {
            next();
            while (!is("punc", "]")) {
                if (first) {
                    first = false;
                } else {
                    expect(",");
                }

                if (is("expand", "...")) {
                    is_expand = true;
                    expand_token = S.token;
                    used_parameters.mark_spread(S.token);
                    next();
                }
                if (is("punc")) {
                    switch (S.token.value) {
                      case ",":
                        elements.push(new AST_Hole({
                            start: S.token,
                            end: S.token
                        }));
                        continue;
                      case "]": // Trailing comma after last element
                        break;
                      case "[":
                      case "{":
                        elements.push(binding_element(used_parameters, symbol_type));
                        break;
                      default:
                        unexpected();
                    }
                } else if (is("name")) {
                    used_parameters.add_parameter(S.token);
                    elements.push(as_symbol(symbol_type));
                } else {
                    croak("Invalid function parameter");
                }
                if (is("operator", "=") && is_expand === false) {
                    used_parameters.mark_default_assignment(S.token);
                    next();
                    elements[elements.length - 1] = new AST_DefaultAssign({
                        start: elements[elements.length - 1].start,
                        left: elements[elements.length - 1],
                        operator: "=",
                        right: expression(false),
                        end: S.token
                    });
                }
                if (is_expand) {
                    if (!is("punc", "]")) {
                        croak("Rest element must be last element");
                    }
                    elements[elements.length - 1] = new AST_Expansion({
                        start: expand_token,
                        expression: elements[elements.length - 1],
                        end: expand_token
                    });
                }
            }
            expect("]");
            used_parameters.check_strict();
            return new AST_Destructuring({
                start: first_token,
                names: elements,
                is_array: true,
                end: prev()
            });
        } else if (is("punc", "{")) {
            next();
            while (!is("punc", "}")) {
                if (first) {
                    first = false;
                } else {
                    expect(",");
                }
                if (is("expand", "...")) {
                    is_expand = true;
                    expand_token = S.token;
                    used_parameters.mark_spread(S.token);
                    next();
                }
                if (is("name") && (is_token(peek(), "punc") || is_token(peek(), "operator")) && [",", "}", "="].includes(peek().value)) {
                    used_parameters.add_parameter(S.token);
                    var start = prev();
                    var value = as_symbol(symbol_type);
                    if (is_expand) {
                        elements.push(new AST_Expansion({
                            start: expand_token,
                            expression: value,
                            end: value.end,
                        }));
                    } else {
                        elements.push(new AST_ObjectKeyVal({
                            start: start,
                            key: value.name,
                            value: value,
                            end: value.end,
                        }));
                    }
                } else if (is("punc", "}")) {
                    continue; // Allow trailing hole
                } else {
                    var property_token = S.token;
                    var property = as_property_name();
                    if (property === null) {
                        unexpected(prev());
                    } else if (prev().type === "name" && !is("punc", ":")) {
                        elements.push(new AST_ObjectKeyVal({
                            start: prev(),
                            key: property,
                            value: new symbol_type({
                                start: prev(),
                                name: property,
                                end: prev()
                            }),
                            end: prev()
                        }));
                    } else {
                        expect(":");
                        elements.push(new AST_ObjectKeyVal({
                            start: property_token,
                            quote: property_token.quote,
                            key: property,
                            value: binding_element(used_parameters, symbol_type),
                            end: prev()
                        }));
                    }
                }
                if (is_expand) {
                    if (!is("punc", "}")) {
                        croak("Rest element must be last element");
                    }
                } else if (is("operator", "=")) {
                    used_parameters.mark_default_assignment(S.token);
                    next();
                    elements[elements.length - 1].value = new AST_DefaultAssign({
                        start: elements[elements.length - 1].value.start,
                        left: elements[elements.length - 1].value,
                        operator: "=",
                        right: expression(false),
                        end: S.token
                    });
                }
            }
            expect("}");
            used_parameters.check_strict();
            return new AST_Destructuring({
                start: first_token,
                names: elements,
                is_array: false,
                end: prev()
            });
        } else if (is("name")) {
            used_parameters.add_parameter(S.token);
            return as_symbol(symbol_type);
        } else {
            croak("Invalid function parameter");
        }
    }

    function params_or_seq_(allow_arrows, maybe_sequence) {
        var spread_token;
        var invalid_sequence;
        var trailing_comma;
        var a = [];
        expect("(");
        while (!is("punc", ")")) {
            if (spread_token) unexpected(spread_token);
            if (is("expand", "...")) {
                spread_token = S.token;
                if (maybe_sequence) invalid_sequence = S.token;
                next();
                a.push(new AST_Expansion({
                    start: prev(),
                    expression: expression(),
                    end: S.token,
                }));
            } else {
                a.push(expression());
            }
            if (!is("punc", ")")) {
                expect(",");
                if (is("punc", ")")) {
                    trailing_comma = prev();
                    if (maybe_sequence) invalid_sequence = trailing_comma;
                }
            }
        }
        expect(")");
        if (allow_arrows && is("arrow", "=>")) {
            if (spread_token && trailing_comma) unexpected(trailing_comma);
        } else if (invalid_sequence) {
            unexpected(invalid_sequence);
        }
        return a;
    }

    function _function_body(block, generator, is_async, name, args) {
        var loop = S.in_loop;
        var labels = S.labels;
        var current_generator = S.in_generator;
        var current_async = S.in_async;
        ++S.in_function;
        if (generator)
            S.in_generator = S.in_function;
        if (is_async)
            S.in_async = S.in_function;
        if (args) parameters(args);
        if (block)
            S.in_directives = true;
        S.in_loop = 0;
        S.labels = [];
        if (block) {
            S.input.push_directives_stack();
            var a = block_();
            if (name) _verify_symbol(name);
            if (args) args.forEach(_verify_symbol);
            S.input.pop_directives_stack();
        } else {
            var a = [new AST_Return({
                start: S.token,
                value: expression(false),
                end: S.token
            })];
        }
        --S.in_function;
        S.in_loop = loop;
        S.labels = labels;
        S.in_generator = current_generator;
        S.in_async = current_async;
        return a;
    }

    function _await_expression() {
        // Previous token must be "await" and not be interpreted as an identifier
        if (!can_await()) {
            croak("Unexpected await expression outside async function",
                S.prev.line, S.prev.col, S.prev.pos);
        }
        // the await expression is parsed as a unary expression in Babel
        return new AST_Await({
            start: prev(),
            end: S.token,
            expression : maybe_unary(true),
        });
    }

    function _yield_expression() {
        // Previous token must be keyword yield and not be interpret as an identifier
        if (!is_in_generator()) {
            croak("Unexpected yield expression outside generator function",
                S.prev.line, S.prev.col, S.prev.pos);
        }
        var start = S.token;
        var star = false;
        var has_expression = true;

        // Attempt to get expression or star (and then the mandatory expression)
        // behind yield on the same line.
        //
        // If nothing follows on the same line of the yieldExpression,
        // it should default to the value `undefined` for yield to return.
        // In that case, the `undefined` stored as `null` in ast.
        //
        // Note 1: It isn't allowed for yield* to close without an expression
        // Note 2: If there is a nlb between yield and star, it is interpret as
        //         yield <explicit undefined> <inserted automatic semicolon> *
        if (can_insert_semicolon() ||
            (is("punc") && PUNC_AFTER_EXPRESSION.has(S.token.value))) {
            has_expression = false;

        } else if (is("operator", "*")) {
            star = true;
            next();
        }

        return new AST_Yield({
            start      : start,
            is_star    : star,
            expression : has_expression ? expression() : null,
            end        : prev()
        });
    }

    function if_() {
        var cond = parenthesised(), body = statement(false, false, true), belse = null;
        if (is("keyword", "else")) {
            next();
            belse = statement(false, false, true);
        }
        return new AST_If({
            condition   : cond,
            body        : body,
            alternative : belse
        });
    }

    function block_() {
        expect("{");
        var a = [];
        while (!is("punc", "}")) {
            if (is("eof")) unexpected();
            a.push(statement());
        }
        next();
        return a;
    }

    function switch_body_() {
        expect("{");
        var a = [], cur = null, branch = null, tmp;
        while (!is("punc", "}")) {
            if (is("eof")) unexpected();
            if (is("keyword", "case")) {
                if (branch) branch.end = prev();
                cur = [];
                branch = new AST_Case({
                    start      : (tmp = S.token, next(), tmp),
                    expression : expression(true),
                    body       : cur
                });
                a.push(branch);
                expect(":");
            } else if (is("keyword", "default")) {
                if (branch) branch.end = prev();
                cur = [];
                branch = new AST_Default({
                    start : (tmp = S.token, next(), expect(":"), tmp),
                    body  : cur
                });
                a.push(branch);
            } else {
                if (!cur) unexpected();
                cur.push(statement());
            }
        }
        if (branch) branch.end = prev();
        next();
        return a;
    }

    function try_() {
        var body, bcatch = null, bfinally = null;
        body = new AST_TryBlock({
            start : S.token,
            body  : block_(),
            end   : prev(),
        });
        if (is("keyword", "catch")) {
            var start = S.token;
            next();
            if (is("punc", "{")) {
                var name = null;
            } else {
                expect("(");
                var name = parameter(undefined, AST_SymbolCatch);
                expect(")");
            }
            bcatch = new AST_Catch({
                start   : start,
                argname : name,
                body    : block_(),
                end     : prev()
            });
        }
        if (is("keyword", "finally")) {
            var start = S.token;
            next();
            bfinally = new AST_Finally({
                start : start,
                body  : block_(),
                end   : prev()
            });
        }
        if (!bcatch && !bfinally)
            croak("Missing catch/finally blocks");
        return new AST_Try({
            body     : body,
            bcatch   : bcatch,
            bfinally : bfinally
        });
    }

    /**
     * var
     *   vardef1 = 2,
     *   vardef2 = 3;
     */
    function vardefs(no_in, kind) {
        var var_defs = [];
        var def;
        for (;;) {
            var sym_type =
                kind === "var" ? AST_SymbolVar :
                kind === "const" ? AST_SymbolConst :
                kind === "let" ? AST_SymbolLet : null;
            // var { a } = b
            if (is("punc", "{") || is("punc", "[")) {
                def = new AST_VarDef({
                    start: S.token,
                    name: binding_element(undefined, sym_type),
                    value: is("operator", "=") ? (expect_token("operator", "="), expression(false, no_in)) : null,
                    end: prev()
                });
            } else {
                def = new AST_VarDef({
                    start : S.token,
                    name  : as_symbol(sym_type),
                    value : is("operator", "=")
                        ? (next(), expression(false, no_in))
                        : !no_in && kind === "const"
                            ? croak("Missing initializer in const declaration") : null,
                    end   : prev()
                });
                if (def.name.name == "import") croak("Unexpected token: import");
            }
            var_defs.push(def);
            if (!is("punc", ","))
                break;
            next();
        }
        return var_defs;
    }

    var var_ = function(no_in) {
        return new AST_Var({
            start       : prev(),
            definitions : vardefs(no_in, "var"),
            end         : prev()
        });
    };

    var let_ = function(no_in) {
        return new AST_Let({
            start       : prev(),
            definitions : vardefs(no_in, "let"),
            end         : prev()
        });
    };

    var const_ = function(no_in) {
        return new AST_Const({
            start       : prev(),
            definitions : vardefs(no_in, "const"),
            end         : prev()
        });
    };

    var new_ = function(allow_calls) {
        var start = S.token;
        expect_token("operator", "new");
        if (is("punc", ".")) {
            next();
            expect_token("name", "target");
            return subscripts(new AST_NewTarget({
                start : start,
                end   : prev()
            }), allow_calls);
        }
        var newexp = expr_atom(false), args;
        if (is("punc", "(")) {
            next();
            args = expr_list(")", true);
        } else {
            args = [];
        }
        var call = new AST_New({
            start      : start,
            expression : newexp,
            args       : args,
            end        : prev()
        });
        annotate(call);
        return subscripts(call, allow_calls);
    };

    function as_atom_node() {
        var tok = S.token, ret;
        switch (tok.type) {
          case "name":
            ret = _make_symbol(AST_SymbolRef);
            break;
          case "num":
            ret = new AST_Number({
                start: tok,
                end: tok,
                value: tok.value,
                raw: LATEST_RAW
            });
            break;
          case "big_int":
            ret = new AST_BigInt({ start: tok, end: tok, value: tok.value });
            break;
          case "string":
            ret = new AST_String({
                start : tok,
                end   : tok,
                value : tok.value,
                quote : tok.quote
            });
            annotate(ret);
            break;
          case "regexp":
            const [_, source, flags] = tok.value.match(/^\/(.*)\/(\w*)$/);

            ret = new AST_RegExp({ start: tok, end: tok, value: { source, flags } });
            break;
          case "atom":
            switch (tok.value) {
              case "false":
                ret = new AST_False({ start: tok, end: tok });
                break;
              case "true":
                ret = new AST_True({ start: tok, end: tok });
                break;
              case "null":
                ret = new AST_Null({ start: tok, end: tok });
                break;
            }
            break;
        }
        next();
        return ret;
    }

    function to_fun_args(ex, default_seen_above) {
        var insert_default = function(ex, default_value) {
            if (default_value) {
                return new AST_DefaultAssign({
                    start: ex.start,
                    left: ex,
                    operator: "=",
                    right: default_value,
                    end: default_value.end
                });
            }
            return ex;
        };
        if (ex instanceof AST_Object) {
            return insert_default(new AST_Destructuring({
                start: ex.start,
                end: ex.end,
                is_array: false,
                names: ex.properties.map(prop => to_fun_args(prop))
            }), default_seen_above);
        } else if (ex instanceof AST_ObjectKeyVal) {
            ex.value = to_fun_args(ex.value);
            return insert_default(ex, default_seen_above);
        } else if (ex instanceof AST_Hole) {
            return ex;
        } else if (ex instanceof AST_Destructuring) {
            ex.names = ex.names.map(name => to_fun_args(name));
            return insert_default(ex, default_seen_above);
        } else if (ex instanceof AST_SymbolRef) {
            return insert_default(new AST_SymbolFunarg({
                name: ex.name,
                start: ex.start,
                end: ex.end
            }), default_seen_above);
        } else if (ex instanceof AST_Expansion) {
            ex.expression = to_fun_args(ex.expression);
            return insert_default(ex, default_seen_above);
        } else if (ex instanceof AST_Array) {
            return insert_default(new AST_Destructuring({
                start: ex.start,
                end: ex.end,
                is_array: true,
                names: ex.elements.map(elm => to_fun_args(elm))
            }), default_seen_above);
        } else if (ex instanceof AST_Assign) {
            return insert_default(to_fun_args(ex.left, ex.right), default_seen_above);
        } else if (ex instanceof AST_DefaultAssign) {
            ex.left = to_fun_args(ex.left);
            return ex;
        } else {
            croak("Invalid function parameter", ex.start.line, ex.start.col);
        }
    }

    var expr_atom = function(allow_calls, allow_arrows) {
        if (is("operator", "new")) {
            return new_(allow_calls);
        }
        if (is("name", "import") && is_token(peek(), "punc", ".")) {
            return import_meta(allow_calls);
        }
        var start = S.token;
        var peeked;
        var async = is("name", "async")
            && (peeked = peek()).value != "["
            && peeked.type != "arrow"
            && as_atom_node();
        if (is("punc")) {
            switch (S.token.value) {
              case "(":
                if (async && !allow_calls) break;
                var exprs = params_or_seq_(allow_arrows, !async);
                if (allow_arrows && is("arrow", "=>")) {
                    return arrow_function(start, exprs.map(e => to_fun_args(e)), !!async);
                }
                var ex = async ? new AST_Call({
                    expression: async,
                    args: exprs
                }) : exprs.length == 1 ? exprs[0] : new AST_Sequence({
                    expressions: exprs
                });
                if (ex.start) {
                    const outer_comments_before = start.comments_before.length;
                    outer_comments_before_counts.set(start, outer_comments_before);
                    ex.start.comments_before.unshift(...start.comments_before);
                    start.comments_before = ex.start.comments_before;
                    if (outer_comments_before == 0 && start.comments_before.length > 0) {
                        var comment = start.comments_before[0];
                        if (!comment.nlb) {
                            comment.nlb = start.nlb;
                            start.nlb = false;
                        }
                    }
                    start.comments_after = ex.start.comments_after;
                }
                ex.start = start;
                var end = prev();
                if (ex.end) {
                    end.comments_before = ex.end.comments_before;
                    ex.end.comments_after.push(...end.comments_after);
                    end.comments_after = ex.end.comments_after;
                }
                ex.end = end;
                if (ex instanceof AST_Call) annotate(ex);
                return subscripts(ex, allow_calls);
              case "[":
                return subscripts(array_(), allow_calls);
              case "{":
                return subscripts(object_or_destructuring_(), allow_calls);
            }
            if (!async) unexpected();
        }
        if (allow_arrows && is("name") && is_token(peek(), "arrow")) {
            var param = new AST_SymbolFunarg({
                name: S.token.value,
                start: start,
                end: start,
            });
            next();
            return arrow_function(start, [param], !!async);
        }
        if (is("keyword", "function")) {
            next();
            var func = function_(AST_Function, false, !!async);
            func.start = start;
            func.end = prev();
            return subscripts(func, allow_calls);
        }
        if (async) return subscripts(async, allow_calls);
        if (is("keyword", "class")) {
            next();
            var cls = class_(AST_ClassExpression);
            cls.start = start;
            cls.end = prev();
            return subscripts(cls, allow_calls);
        }
        if (is("template_head")) {
            return subscripts(template_string(), allow_calls);
        }
        if (is("privatename")) {
            if(!S.in_class) {
                croak("Private field must be used in an enclosing class");
            }

            const start = S.token;
            const key = new AST_SymbolPrivateProperty({
                start,
                name: start.value,
                end: start
            });
            next();
            expect_token("operator", "in");

            const private_in = new AST_PrivateIn({
                start,
                key,
                value: subscripts(as_atom_node(), allow_calls),
                end: prev()
            });

            return subscripts(private_in, allow_calls);
        }
        if (ATOMIC_START_TOKEN.has(S.token.type)) {
            return subscripts(as_atom_node(), allow_calls);
        }
        unexpected();
    };

    function template_string() {
        var segments = [], start = S.token;

        segments.push(new AST_TemplateSegment({
            start: S.token,
            raw: TEMPLATE_RAWS.get(S.token),
            value: S.token.value,
            end: S.token
        }));

        while (!S.token.template_end) {
            next();
            handle_regexp();
            segments.push(expression(true));

            segments.push(new AST_TemplateSegment({
                start: S.token,
                raw: TEMPLATE_RAWS.get(S.token),
                value: S.token.value,
                end: S.token
            }));
        }
        next();

        return new AST_TemplateString({
            start: start,
            segments: segments,
            end: S.token
        });
    }

    function expr_list(closing, allow_trailing_comma, allow_empty) {
        var first = true, a = [];
        while (!is("punc", closing)) {
            if (first) first = false; else expect(",");
            if (allow_trailing_comma && is("punc", closing)) break;
            if (is("punc", ",") && allow_empty) {
                a.push(new AST_Hole({ start: S.token, end: S.token }));
            } else if (is("expand", "...")) {
                next();
                a.push(new AST_Expansion({start: prev(), expression: expression(),end: S.token}));
            } else {
                a.push(expression(false));
            }
        }
        next();
        return a;
    }

    var array_ = embed_tokens(function() {
        expect("[");
        return new AST_Array({
            elements: expr_list("]", !options.strict, true)
        });
    });

    var create_accessor = embed_tokens((is_generator, is_async) => {
        return function_(AST_Accessor, is_generator, is_async);
    });

    var object_or_destructuring_ = embed_tokens(function object_or_destructuring_() {
        var start = S.token, first = true, a = [];
        expect("{");
        while (!is("punc", "}")) {
            if (first) first = false; else expect(",");
            if (!options.strict && is("punc", "}"))
                // allow trailing comma
                break;

            start = S.token;
            if (start.type == "expand") {
                next();
                a.push(new AST_Expansion({
                    start: start,
                    expression: expression(false),
                    end: prev(),
                }));
                continue;
            }
            if(is("privatename")) {
                croak("private fields are not allowed in an object");
            }
            var name = as_property_name();
            var value;

            // Check property and fetch value
            if (!is("punc", ":")) {
                var concise = concise_method_or_getset(name, start);
                if (concise) {
                    a.push(concise);
                    continue;
                }

                value = new AST_SymbolRef({
                    start: prev(),
                    name: name,
                    end: prev()
                });
            } else if (name === null) {
                unexpected(prev());
            } else {
                next(); // `:` - see first condition
                value = expression(false);
            }

            // Check for default value and alter value accordingly if necessary
            if (is("operator", "=")) {
                next();
                value = new AST_Assign({
                    start: start,
                    left: value,
                    operator: "=",
                    right: expression(false),
                    logical: false,
                    end: prev()
                });
            }

            // Create property
            const kv = new AST_ObjectKeyVal({
                start: start,
                quote: start.quote,
                key: name instanceof AST_Node ? name : "" + name,
                value: value,
                end: prev()
            });
            a.push(annotate(kv));
        }
        next();
        return new AST_Object({ properties: a });
    });

    function class_(KindOfClass, is_export_default) {
        var start, method, class_name, extends_, a = [];

        S.input.push_directives_stack(); // Push directive stack, but not scope stack
        S.input.add_directive("use strict");

        if (S.token.type == "name" && S.token.value != "extends") {
            class_name = as_symbol(KindOfClass === AST_DefClass ? AST_SymbolDefClass : AST_SymbolClass);
        }

        if (KindOfClass === AST_DefClass && !class_name) {
            if (is_export_default) {
                KindOfClass = AST_ClassExpression;
            } else {
                unexpected();
            }
        }

        if (S.token.value == "extends") {
            next();
            extends_ = expression(true);
        }

        expect("{");
        // mark in class feild,
        const save_in_class = S.in_class;
        S.in_class = true;
        while (is("punc", ";")) { next(); }  // Leading semicolons are okay in class bodies.
        while (!is("punc", "}")) {
            start = S.token;
            method = concise_method_or_getset(as_property_name(), start, true);
            if (!method) { unexpected(); }
            a.push(method);
            while (is("punc", ";")) { next(); }
        }
        // mark in class feild,
        S.in_class = save_in_class;

        S.input.pop_directives_stack();

        next();

        return new KindOfClass({
            start: start,
            name: class_name,
            extends: extends_,
            properties: a,
            end: prev(),
        });
    }

    function concise_method_or_getset(name, start, is_class) {
        const get_symbol_ast = (name, SymbolClass = AST_SymbolMethod) => {
            if (typeof name === "string" || typeof name === "number") {
                return new SymbolClass({
                    start,
                    name: "" + name,
                    end: prev()
                });
            } else if (name === null) {
                unexpected();
            }
            return name;
        };

        const is_not_method_start = () =>
            !is("punc", "(") && !is("punc", ",") && !is("punc", "}") && !is("punc", ";") && !is("operator", "=");

        var is_async = false;
        var is_static = false;
        var is_generator = false;
        var is_private = false;
        var accessor_type = null;

        if (is_class && name === "static" && is_not_method_start()) {
            const static_block = class_static_block();
            if (static_block != null) {
                return static_block;
            }
            is_static = true;
            name = as_property_name();
        }
        if (name === "async" && is_not_method_start()) {
            is_async = true;
            name = as_property_name();
        }
        if (prev().type === "operator" && prev().value === "*") {
            is_generator = true;
            name = as_property_name();
        }
        if ((name === "get" || name === "set") && is_not_method_start()) {
            accessor_type = name;
            name = as_property_name();
        }
        if (prev().type === "privatename") {
            is_private = true;
        }

        const property_token = prev();

        if (accessor_type != null) {
            if (!is_private) {
                const AccessorClass = accessor_type === "get"
                    ? AST_ObjectGetter
                    : AST_ObjectSetter;

                name = get_symbol_ast(name);
                return annotate(new AccessorClass({
                    start,
                    static: is_static,
                    key: name,
                    quote: name instanceof AST_SymbolMethod ? property_token.quote : undefined,
                    value: create_accessor(),
                    end: prev()
                }));
            } else {
                const AccessorClass = accessor_type === "get"
                    ? AST_PrivateGetter
                    : AST_PrivateSetter;

                return annotate(new AccessorClass({
                    start,
                    static: is_static,
                    key: get_symbol_ast(name),
                    value: create_accessor(),
                    end: prev(),
                }));
            }
        }

        if (is("punc", "(")) {
            name = get_symbol_ast(name);
            const AST_MethodVariant = is_private
                ? AST_PrivateMethod
                : AST_ConciseMethod;
            var node = new AST_MethodVariant({
                start       : start,
                static      : is_static,
                is_generator: is_generator,
                async       : is_async,
                key         : name,
                quote       : name instanceof AST_SymbolMethod ?
                              property_token.quote : undefined,
                value       : create_accessor(is_generator, is_async),
                end         : prev()
            });
            return annotate(node);
        }

        if (is_class) {
            const key = get_symbol_ast(name, AST_SymbolClassProperty);
            const quote = key instanceof AST_SymbolClassProperty
                ? property_token.quote
                : undefined;
            const AST_ClassPropertyVariant = is_private
                ? AST_ClassPrivateProperty
                : AST_ClassProperty;
            if (is("operator", "=")) {
                next();
                return annotate(
                    new AST_ClassPropertyVariant({
                        start,
                        static: is_static,
                        quote,
                        key,
                        value: expression(false),
                        end: prev()
                    })
                );
            } else if (
                is("name")
                || is("privatename")
                || is("operator", "*")
                || is("punc", ";")
                || is("punc", "}")
            ) {
                return annotate(
                    new AST_ClassPropertyVariant({
                        start,
                        static: is_static,
                        quote,
                        key,
                        end: prev()
                    })
                );
            }
        }
    }

    function class_static_block() {
        if (!is("punc", "{")) {
            return null;
        }

        const start = S.token;
        const body = [];

        next();

        while (!is("punc", "}")) {
            body.push(statement());
        }

        next();

        return new AST_ClassStaticBlock({ start, body, end: prev() });
    }

    function maybe_import_assertion() {
        if (is("name", "assert") && !has_newline_before(S.token)) {
            next();
            return object_or_destructuring_();
        }
        return null;
    }

    function import_statement() {
        var start = prev();

        var imported_name;
        var imported_names;
        if (is("name")) {
            imported_name = as_symbol(AST_SymbolImport);
        }

        if (is("punc", ",")) {
            next();
        }

        imported_names = map_names(true);

        if (imported_names || imported_name) {
            expect_token("name", "from");
        }
        var mod_str = S.token;
        if (mod_str.type !== "string") {
            unexpected();
        }
        next();

        const assert_clause = maybe_import_assertion();

        return new AST_Import({
            start,
            imported_name,
            imported_names,
            module_name: new AST_String({
                start: mod_str,
                value: mod_str.value,
                quote: mod_str.quote,
                end: mod_str,
            }),
            assert_clause,
            end: S.token,
        });
    }

    function import_meta(allow_calls) {
        var start = S.token;
        expect_token("name", "import");
        expect_token("punc", ".");
        expect_token("name", "meta");
        return subscripts(new AST_ImportMeta({
            start: start,
            end: prev()
        }), allow_calls);
    }

    function map_name(is_import) {
        function make_symbol(type, quote) {
            return new type({
                name: as_property_name(),
                quote: quote || undefined,
                start: prev(),
                end: prev()
            });
        }

        var foreign_type = is_import ? AST_SymbolImportForeign : AST_SymbolExportForeign;
        var type = is_import ? AST_SymbolImport : AST_SymbolExport;
        var start = S.token;
        var foreign_name;
        var name;

        if (is_import) {
            foreign_name = make_symbol(foreign_type, start.quote);
        } else {
            name = make_symbol(type, start.quote);
        }
        if (is("name", "as")) {
            next();  // The "as" word
            if (is_import) {
                name = make_symbol(type);
            } else {
                foreign_name = make_symbol(foreign_type, S.token.quote);
            }
        } else if (is_import) {
            name = new type(foreign_name);
        } else {
            foreign_name = new foreign_type(name);
        }

        return new AST_NameMapping({
            start: start,
            foreign_name: foreign_name,
            name: name,
            end: prev(),
        });
    }

    function map_nameAsterisk(is_import, import_or_export_foreign_name) {
        var foreign_type = is_import ? AST_SymbolImportForeign : AST_SymbolExportForeign;
        var type = is_import ? AST_SymbolImport : AST_SymbolExport;
        var start = S.token;
        var name, foreign_name;
        var end = prev();

        if (is_import) {
            name = import_or_export_foreign_name;
        } else {
            foreign_name = import_or_export_foreign_name;
        }

        name = name || new type({
            start: start,
            name: "*",
            end: end,
        });

        foreign_name = foreign_name || new foreign_type({
            start: start,
            name: "*",
            end: end,
        });

        return new AST_NameMapping({
            start: start,
            foreign_name: foreign_name,
            name: name,
            end: end,
        });
    }

    function map_names(is_import) {
        var names;
        if (is("punc", "{")) {
            next();
            names = [];
            while (!is("punc", "}")) {
                names.push(map_name(is_import));
                if (is("punc", ",")) {
                    next();
                }
            }
            next();
        } else if (is("operator", "*")) {
            var name;
            next();
            if (is("name", "as")) {
                next();  // The "as" word
                name = is_import ? as_symbol(AST_SymbolImport) : as_symbol_or_string(AST_SymbolExportForeign);
            }
            names = [map_nameAsterisk(is_import, name)];
        }
        return names;
    }

    function export_statement() {
        var start = S.token;
        var is_default;
        var exported_names;

        if (is("keyword", "default")) {
            is_default = true;
            next();
        } else if (exported_names = map_names(false)) {
            if (is("name", "from")) {
                next();

                var mod_str = S.token;
                if (mod_str.type !== "string") {
                    unexpected();
                }
                next();

                const assert_clause = maybe_import_assertion();

                return new AST_Export({
                    start: start,
                    is_default: is_default,
                    exported_names: exported_names,
                    module_name: new AST_String({
                        start: mod_str,
                        value: mod_str.value,
                        quote: mod_str.quote,
                        end: mod_str,
                    }),
                    end: prev(),
                    assert_clause
                });
            } else {
                return new AST_Export({
                    start: start,
                    is_default: is_default,
                    exported_names: exported_names,
                    end: prev(),
                });
            }
        }

        var node;
        var exported_value;
        var exported_definition;
        if (is("punc", "{")
            || is_default
                && (is("keyword", "class") || is("keyword", "function"))
                && is_token(peek(), "punc")) {
            exported_value = expression(false);
            semicolon();
        } else if ((node = statement(is_default)) instanceof AST_Definitions && is_default) {
            unexpected(node.start);
        } else if (
            node instanceof AST_Definitions
            || node instanceof AST_Defun
            || node instanceof AST_DefClass
        ) {
            exported_definition = node;
        } else if (
            node instanceof AST_ClassExpression
            || node instanceof AST_Function
        ) {
            exported_value = node;
        } else if (node instanceof AST_SimpleStatement) {
            exported_value = node.body;
        } else {
            unexpected(node.start);
        }

        return new AST_Export({
            start: start,
            is_default: is_default,
            exported_value: exported_value,
            exported_definition: exported_definition,
            end: prev(),
            assert_clause: null
        });
    }

    function as_property_name() {
        var tmp = S.token;
        switch (tmp.type) {
          case "punc":
            if (tmp.value === "[") {
                next();
                var ex = expression(false);
                expect("]");
                return ex;
            } else unexpected(tmp);
          case "operator":
            if (tmp.value === "*") {
                next();
                return null;
            }
            if (!["delete", "in", "instanceof", "new", "typeof", "void"].includes(tmp.value)) {
                unexpected(tmp);
            }
            /* falls through */
          case "name":
          case "privatename":
          case "string":
          case "num":
          case "big_int":
          case "keyword":
          case "atom":
            next();
            return tmp.value;
          default:
            unexpected(tmp);
        }
    }

    function as_name() {
        var tmp = S.token;
        if (tmp.type != "name" && tmp.type != "privatename") unexpected();
        next();
        return tmp.value;
    }

    function _make_symbol(type) {
        var name = S.token.value;
        return new (name == "this" ? AST_This :
                    name == "super" ? AST_Super :
                    type)({
            name  : String(name),
            start : S.token,
            end   : S.token
        });
    }

    function _verify_symbol(sym) {
        var name = sym.name;
        if (is_in_generator() && name == "yield") {
            token_error(sym.start, "Yield cannot be used as identifier inside generators");
        }
        if (S.input.has_directive("use strict")) {
            if (name == "yield") {
                token_error(sym.start, "Unexpected yield identifier inside strict mode");
            }
            if (sym instanceof AST_SymbolDeclaration && (name == "arguments" || name == "eval")) {
                token_error(sym.start, "Unexpected " + name + " in strict mode");
            }
        }
    }

    function as_symbol(type, noerror) {
        if (!is("name")) {
            if (!noerror) croak("Name expected");
            return null;
        }
        var sym = _make_symbol(type);
        _verify_symbol(sym);
        next();
        return sym;
    }

    function as_symbol_or_string(type) {
        if (!is("name")) {
            if (!is("string")) {
                croak("Name or string expected");
            }
            var tok = S.token;
            var ret = new type({
                start : tok,
                end   : tok,
                name : tok.value,
                quote : tok.quote
            });
            next();
            return ret;
        }
        var sym = _make_symbol(type);
        _verify_symbol(sym);
        next();
        return sym;
    }

    // Annotate AST_Call, AST_Lambda or AST_New with the special comments
    function annotate(node, before_token = node.start) {
        var comments = before_token.comments_before;
        const comments_outside_parens = outer_comments_before_counts.get(before_token);
        var i = comments_outside_parens != null ? comments_outside_parens : comments.length;
        while (--i >= 0) {
            var comment = comments[i];
            if (/[@#]__/.test(comment.value)) {
                if (/[@#]__PURE__/.test(comment.value)) {
                    set_annotation(node, _PURE);
                    break;
                }
                if (/[@#]__INLINE__/.test(comment.value)) {
                    set_annotation(node, _INLINE);
                    break;
                }
                if (/[@#]__NOINLINE__/.test(comment.value)) {
                    set_annotation(node, _NOINLINE);
                    break;
                }
                if (/[@#]__KEY__/.test(comment.value)) {
                    set_annotation(node, _KEY);
                    break;
                }
                if (/[@#]__MANGLE_PROP__/.test(comment.value)) {
                    set_annotation(node, _MANGLEPROP);
                    break;
                }
            }
        }
        return node;
    }

    var subscripts = function(expr, allow_calls, is_chain) {
        var start = expr.start;
        if (is("punc", ".")) {
            next();
            if(is("privatename") && !S.in_class) 
                croak("Private field must be used in an enclosing class");
            const AST_DotVariant = is("privatename") ? AST_DotHash : AST_Dot;
            return subscripts(new AST_DotVariant({
                start      : start,
                expression : expr,
                optional   : false,
                property   : as_name(),
                end        : prev()
            }), allow_calls, is_chain);
        }
        if (is("punc", "[")) {
            next();
            var prop = expression(true);
            expect("]");
            return subscripts(new AST_Sub({
                start      : start,
                expression : expr,
                optional   : false,
                property   : prop,
                end        : prev()
            }), allow_calls, is_chain);
        }
        if (allow_calls && is("punc", "(")) {
            next();
            var call = new AST_Call({
                start      : start,
                expression : expr,
                optional   : false,
                args       : call_args(),
                end        : prev()
            });
            annotate(call);
            return subscripts(call, true, is_chain);
        }

        if (is("punc", "?.")) {
            next();

            let chain_contents;

            if (allow_calls && is("punc", "(")) {
                next();

                const call = new AST_Call({
                    start,
                    optional: true,
                    expression: expr,
                    args: call_args(),
                    end: prev()
                });
                annotate(call);

                chain_contents = subscripts(call, true, true);
            } else if (is("name") || is("privatename")) {
                if(is("privatename") && !S.in_class) 
                    croak("Private field must be used in an enclosing class");
                const AST_DotVariant = is("privatename") ? AST_DotHash : AST_Dot;
                chain_contents = subscripts(new AST_DotVariant({
                    start,
                    expression: expr,
                    optional: true,
                    property: as_name(),
                    end: prev()
                }), allow_calls, true);
            } else if (is("punc", "[")) {
                next();
                const property = expression(true);
                expect("]");
                chain_contents = subscripts(new AST_Sub({
                    start,
                    expression: expr,
                    optional: true,
                    property,
                    end: prev()
                }), allow_calls, true);
            }

            if (!chain_contents) unexpected();

            if (chain_contents instanceof AST_Chain) return chain_contents;

            return new AST_Chain({
                start,
                expression: chain_contents,
                end: prev()
            });
        }

        if (is("template_head")) {
            if (is_chain) {
                // a?.b`c` is a syntax error
                unexpected();
            }

            return subscripts(new AST_PrefixedTemplateString({
                start: start,
                prefix: expr,
                template_string: template_string(),
                end: prev()
            }), allow_calls);
        }
        return expr;
    };

    function call_args() {
        var args = [];
        while (!is("punc", ")")) {
            if (is("expand", "...")) {
                next();
                args.push(new AST_Expansion({
                    start: prev(),
                    expression: expression(false),
                    end: prev()
                }));
            } else {
                args.push(expression(false));
            }
            if (!is("punc", ")")) {
                expect(",");
            }
        }
        next();
        return args;
    }

    var maybe_unary = function(allow_calls, allow_arrows) {
        var start = S.token;
        if (start.type == "name" && start.value == "await" && can_await()) {
            next();
            return _await_expression();
        }
        if (is("operator") && UNARY_PREFIX.has(start.value)) {
            next();
            handle_regexp();
            var ex = make_unary(AST_UnaryPrefix, start, maybe_unary(allow_calls));
            ex.start = start;
            ex.end = prev();
            return ex;
        }
        var val = expr_atom(allow_calls, allow_arrows);
        while (is("operator") && UNARY_POSTFIX.has(S.token.value) && !has_newline_before(S.token)) {
            if (val instanceof AST_Arrow) unexpected();
            val = make_unary(AST_UnaryPostfix, S.token, val);
            val.start = start;
            val.end = S.token;
            next();
        }
        return val;
    };

    function make_unary(ctor, token, expr) {
        var op = token.value;
        switch (op) {
          case "++":
          case "--":
            if (!is_assignable(expr))
                croak("Invalid use of " + op + " operator", token.line, token.col, token.pos);
            break;
          case "delete":
            if (expr instanceof AST_SymbolRef && S.input.has_directive("use strict"))
                croak("Calling delete on expression not allowed in strict mode", expr.start.line, expr.start.col, expr.start.pos);
            break;
        }
        return new ctor({ operator: op, expression: expr });
    }

    var expr_op = function(left, min_prec, no_in) {
        var op = is("operator") ? S.token.value : null;
        if (op == "in" && no_in) op = null;
        if (op == "**" && left instanceof AST_UnaryPrefix
            /* unary token in front not allowed - parenthesis required */
            && !is_token(left.start, "punc", "(")
            && left.operator !== "--" && left.operator !== "++")
                unexpected(left.start);
        var prec = op != null ? PRECEDENCE[op] : null;
        if (prec != null && (prec > min_prec || (op === "**" && min_prec === prec))) {
            next();
            var right = expr_op(maybe_unary(true), prec, no_in);
            return expr_op(new AST_Binary({
                start    : left.start,
                left     : left,
                operator : op,
                right    : right,
                end      : right.end
            }), min_prec, no_in);
        }
        return left;
    };

    function expr_ops(no_in) {
        return expr_op(maybe_unary(true, true), 0, no_in);
    }

    var maybe_conditional = function(no_in) {
        var start = S.token;
        var expr = expr_ops(no_in);
        if (is("operator", "?")) {
            next();
            var yes = expression(false);
            expect(":");
            return new AST_Conditional({
                start       : start,
                condition   : expr,
                consequent  : yes,
                alternative : expression(false, no_in),
                end         : prev()
            });
        }
        return expr;
    };

    function is_assignable(expr) {
        return expr instanceof AST_PropAccess || expr instanceof AST_SymbolRef;
    }

    function to_destructuring(node) {
        if (node instanceof AST_Object) {
            node = new AST_Destructuring({
                start: node.start,
                names: node.properties.map(to_destructuring),
                is_array: false,
                end: node.end
            });
        } else if (node instanceof AST_Array) {
            var names = [];

            for (var i = 0; i < node.elements.length; i++) {
                // Only allow expansion as last element
                if (node.elements[i] instanceof AST_Expansion) {
                    if (i + 1 !== node.elements.length) {
                        token_error(node.elements[i].start, "Spread must the be last element in destructuring array");
                    }
                    node.elements[i].expression = to_destructuring(node.elements[i].expression);
                }

                names.push(to_destructuring(node.elements[i]));
            }

            node = new AST_Destructuring({
                start: node.start,
                names: names,
                is_array: true,
                end: node.end
            });
        } else if (node instanceof AST_ObjectProperty) {
            node.value = to_destructuring(node.value);
        } else if (node instanceof AST_Assign) {
            node = new AST_DefaultAssign({
                start: node.start,
                left: node.left,
                operator: "=",
                right: node.right,
                end: node.end
            });
        }
        return node;
    }

    // In ES6, AssignmentExpression can also be an ArrowFunction
    var maybe_assign = function(no_in) {
        handle_regexp();
        var start = S.token;

        if (start.type == "name" && start.value == "yield") {
            if (is_in_generator()) {
                next();
                return _yield_expression();
            } else if (S.input.has_directive("use strict")) {
                token_error(S.token, "Unexpected yield identifier inside strict mode");
            }
        }

        var left = maybe_conditional(no_in);
        var val = S.token.value;

        if (is("operator") && ASSIGNMENT.has(val)) {
            if (is_assignable(left) || (left = to_destructuring(left)) instanceof AST_Destructuring) {
                next();

                return new AST_Assign({
                    start    : start,
                    left     : left,
                    operator : val,
                    right    : maybe_assign(no_in),
                    logical  : LOGICAL_ASSIGNMENT.has(val),
                    end      : prev()
                });
            }
            croak("Invalid assignment");
        }
        return left;
    };

    var expression = function(commas, no_in) {
        var start = S.token;
        var exprs = [];
        while (true) {
            exprs.push(maybe_assign(no_in));
            if (!commas || !is("punc", ",")) break;
            next();
            commas = true;
        }
        return exprs.length == 1 ? exprs[0] : new AST_Sequence({
            start       : start,
            expressions : exprs,
            end         : peek()
        });
    };

    function in_loop(cont) {
        ++S.in_loop;
        var ret = cont();
        --S.in_loop;
        return ret;
    }

    if (options.expression) {
        return expression(true);
    }

    return (function parse_toplevel() {
        var start = S.token;
        var body = [];
        S.input.push_directives_stack();
        if (options.module) S.input.add_directive("use strict");
        while (!is("eof")) {
            body.push(statement());
        }
        S.input.pop_directives_stack();
        var end = prev();
        var toplevel = options.toplevel;
        if (toplevel) {
            toplevel.body = toplevel.body.concat(body);
            toplevel.end = end;
        } else {
            toplevel = new AST_Toplevel({ start: start, body: body, end: end });
        }
        TEMPLATE_RAWS = new Map();
        return toplevel;
    })();

}

/***********************************************************************

  A JavaScript tokenizer / parser / beautifier / compressor.
  https://github.com/mishoo/UglifyJS2

  -------------------------------- (C) ---------------------------------

                           Author: Mihai Bazon
                         <mihai.bazon@gmail.com>
                       http://mihai.bazon.net/blog

  Distributed under the BSD license:

    Copyright 2012 (c) Mihai Bazon <mihai.bazon@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        * Redistributions of source code must retain the above
          copyright notice, this list of conditions and the following
          disclaimer.

        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials
          provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

 ***********************************************************************/

function DEFNODE(type, props, ctor, methods, base = AST_Node) {
    if (!props) props = [];
    else props = props.split(/\s+/);
    var self_props = props;
    if (base && base.PROPS)
        props = props.concat(base.PROPS);
    const proto = base && Object.create(base.prototype);
    if (proto) {
        ctor.prototype = proto;
        ctor.BASE = base;
    }
    if (base) base.SUBCLASSES.push(ctor);
    ctor.prototype.CTOR = ctor;
    ctor.prototype.constructor = ctor;
    ctor.PROPS = props || null;
    ctor.SELF_PROPS = self_props;
    ctor.SUBCLASSES = [];
    if (type) {
        ctor.prototype.TYPE = ctor.TYPE = type;
    }
    if (methods) for (let i in methods) if (HOP(methods, i)) {
        if (i[0] === "$") {
            ctor[i.substr(1)] = methods[i];
        } else {
            ctor.prototype[i] = methods[i];
        }
    }
    ctor.DEFMETHOD = function(name, method) {
        this.prototype[name] = method;
    };
    return ctor;
}

const has_tok_flag = (tok, flag) => Boolean(tok.flags & flag);
const set_tok_flag = (tok, flag, truth) => {
    if (truth) {
        tok.flags |= flag;
    } else {
        tok.flags &= ~flag;
    }
};

const TOK_FLAG_NLB          = 0b0001;
const TOK_FLAG_QUOTE_SINGLE = 0b0010;
const TOK_FLAG_QUOTE_EXISTS = 0b0100;
const TOK_FLAG_TEMPLATE_END = 0b1000;

class AST_Token {
    constructor(type, value, line, col, pos, nlb, comments_before, comments_after, file) {
        this.flags = (nlb ? 1 : 0);

        this.type = type;
        this.value = value;
        this.line = line;
        this.col = col;
        this.pos = pos;
        this.comments_before = comments_before;
        this.comments_after = comments_after;
        this.file = file;

        Object.seal(this);
    }

    // Return a string summary of the token for node.js console.log
    [Symbol.for("nodejs.util.inspect.custom")](_depth, options) {
        const special = str => options.stylize(str, "special");
        const quote = typeof this.value === "string" && this.value.includes("`") ? "'" : "`";
        const value = `${quote}${this.value}${quote}`;
        return `${special("[AST_Token")} ${value} at ${this.line}:${this.col}${special("]")}`;
    }

    get nlb() {
        return has_tok_flag(this, TOK_FLAG_NLB);
    }

    set nlb(new_nlb) {
        set_tok_flag(this, TOK_FLAG_NLB, new_nlb);
    }

    get quote() {
        return !has_tok_flag(this, TOK_FLAG_QUOTE_EXISTS)
            ? ""
            : (has_tok_flag(this, TOK_FLAG_QUOTE_SINGLE) ? "'" : '"');
    }

    set quote(quote_type) {
        set_tok_flag(this, TOK_FLAG_QUOTE_SINGLE, quote_type === "'");
        set_tok_flag(this, TOK_FLAG_QUOTE_EXISTS, !!quote_type);
    }

    get template_end() {
        return has_tok_flag(this, TOK_FLAG_TEMPLATE_END);
    }

    set template_end(new_template_end) {
        set_tok_flag(this, TOK_FLAG_TEMPLATE_END, new_template_end);
    }
}

var AST_Node = DEFNODE("Node", "start end", function AST_Node(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    _clone: function(deep) {
        if (deep) {
            var self = this.clone();
            return self.transform(new TreeTransformer(function(node) {
                if (node !== self) {
                    return node.clone(true);
                }
            }));
        }
        return new this.CTOR(this);
    },
    clone: function(deep) {
        return this._clone(deep);
    },
    $documentation: "Base class of all AST nodes",
    $propdoc: {
        start: "[AST_Token] The first token of this node",
        end: "[AST_Token] The last token of this node"
    },
    _walk: function(visitor) {
        return visitor._visit(this);
    },
    walk: function(visitor) {
        return this._walk(visitor); // not sure the indirection will be any help
    },
    _children_backwards: () => {}
}, null);

/* -----[ statements ]----- */

var AST_Statement = DEFNODE("Statement", null, function AST_Statement(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class of all statements",
});

var AST_Debugger = DEFNODE("Debugger", null, function AST_Debugger(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Represents a debugger statement",
}, AST_Statement);

var AST_Directive = DEFNODE("Directive", "value quote", function AST_Directive(props) {
    if (props) {
        this.value = props.value;
        this.quote = props.quote;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Represents a directive, like \"use strict\";",
    $propdoc: {
        value: "[string] The value of this directive as a plain string (it's not an AST_String!)",
        quote: "[string] the original quote character"
    },
}, AST_Statement);

var AST_SimpleStatement = DEFNODE("SimpleStatement", "body", function AST_SimpleStatement(props) {
    if (props) {
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A statement consisting of an expression, i.e. a = 1 + 2",
    $propdoc: {
        body: "[AST_Node] an expression node (should not be instanceof AST_Statement)"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.body._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.body);
    }
}, AST_Statement);

// XXX Emscripten localmod: Add a node type for a parenthesized expression so that we can retain
// Closure annotations that need a form "/**annotation*/(expression)"
var AST_ParenthesizedExpression = DEFNODE("ParenthesizedExpression", "body", function AST_ParenthesizedExpression(props) {
    if (props) {
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An explicitly parenthesized expression, i.e. a = (1 + 2)",
    $propdoc: {
        body: "[AST_Node] an expression node (should not be instanceof AST_Statement)"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.body._walk(visitor);
        });
    }
}, AST_Statement);
// XXX End of Emscripten localmod

function walk_body(node, visitor) {
    const body = node.body;
    for (var i = 0, len = body.length; i < len; i++) {
        body[i]._walk(visitor);
    }
}

function clone_block_scope(deep) {
    var clone = this._clone(deep);
    if (this.block_scope) {
        clone.block_scope = this.block_scope.clone();
    }
    return clone;
}

var AST_Block = DEFNODE("Block", "body block_scope", function AST_Block(props) {
    if (props) {
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A body of statements (usually braced)",
    $propdoc: {
        body: "[AST_Statement*] an array of statements",
        block_scope: "[AST_Scope] the block scope"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            walk_body(this, visitor);
        });
    },
    _children_backwards(push) {
        let i = this.body.length;
        while (i--) push(this.body[i]);
    },
    clone: clone_block_scope
}, AST_Statement);

var AST_BlockStatement = DEFNODE("BlockStatement", null, function AST_BlockStatement(props) {
    if (props) {
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A block statement",
}, AST_Block);

var AST_EmptyStatement = DEFNODE("EmptyStatement", null, function AST_EmptyStatement(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The empty statement (empty block or simply a semicolon)"
}, AST_Statement);

var AST_StatementWithBody = DEFNODE("StatementWithBody", "body", function AST_StatementWithBody(props) {
    if (props) {
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for all statements that contain one nested body: `For`, `ForIn`, `Do`, `While`, `With`",
    $propdoc: {
        body: "[AST_Statement] the body; this should always be present, even if it's an AST_EmptyStatement"
    }
}, AST_Statement);

var AST_LabeledStatement = DEFNODE("LabeledStatement", "label", function AST_LabeledStatement(props) {
    if (props) {
        this.label = props.label;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Statement with a label",
    $propdoc: {
        label: "[AST_Label] a label definition"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.label._walk(visitor);
            this.body._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.body);
        push(this.label);
    },
    clone: function(deep) {
        var node = this._clone(deep);
        if (deep) {
            var label = node.label;
            var def = this.label;
            node.walk(new TreeWalker(function(node) {
                if (node instanceof AST_LoopControl
                    && node.label && node.label.thedef === def) {
                    node.label.thedef = label;
                    label.references.push(node);
                }
            }));
        }
        return node;
    }
}, AST_StatementWithBody);

var AST_IterationStatement = DEFNODE(
    "IterationStatement",
    "block_scope",
    function AST_IterationStatement(props) {
        if (props) {
            this.block_scope = props.block_scope;
            this.body = props.body;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "Internal class.  All loops inherit from it.",
        $propdoc: {
            block_scope: "[AST_Scope] the block scope for this iteration statement."
        },
        clone: clone_block_scope
    },
    AST_StatementWithBody
);

var AST_DWLoop = DEFNODE("DWLoop", "condition", function AST_DWLoop(props) {
    if (props) {
        this.condition = props.condition;
        this.block_scope = props.block_scope;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for do/while statements",
    $propdoc: {
        condition: "[AST_Node] the loop condition.  Should not be instanceof AST_Statement"
    }
}, AST_IterationStatement);

var AST_Do = DEFNODE("Do", null, function AST_Do(props) {
    if (props) {
        this.condition = props.condition;
        this.block_scope = props.block_scope;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `do` statement",
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.body._walk(visitor);
            this.condition._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.condition);
        push(this.body);
    }
}, AST_DWLoop);

var AST_While = DEFNODE("While", null, function AST_While(props) {
    if (props) {
        this.condition = props.condition;
        this.block_scope = props.block_scope;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `while` statement",
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.condition._walk(visitor);
            this.body._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.body);
        push(this.condition);
    },
}, AST_DWLoop);

var AST_For = DEFNODE("For", "init condition step", function AST_For(props) {
    if (props) {
        this.init = props.init;
        this.condition = props.condition;
        this.step = props.step;
        this.block_scope = props.block_scope;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `for` statement",
    $propdoc: {
        init: "[AST_Node?] the `for` initialization code, or null if empty",
        condition: "[AST_Node?] the `for` termination clause, or null if empty",
        step: "[AST_Node?] the `for` update clause, or null if empty"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            if (this.init) this.init._walk(visitor);
            if (this.condition) this.condition._walk(visitor);
            if (this.step) this.step._walk(visitor);
            this.body._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.body);
        if (this.step) push(this.step);
        if (this.condition) push(this.condition);
        if (this.init) push(this.init);
    },
}, AST_IterationStatement);

var AST_ForIn = DEFNODE("ForIn", "init object", function AST_ForIn(props) {
    if (props) {
        this.init = props.init;
        this.object = props.object;
        this.block_scope = props.block_scope;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `for ... in` statement",
    $propdoc: {
        init: "[AST_Node] the `for/in` initialization code",
        object: "[AST_Node] the object that we're looping through"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.init._walk(visitor);
            this.object._walk(visitor);
            this.body._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.body);
        if (this.object) push(this.object);
        if (this.init) push(this.init);
    },
}, AST_IterationStatement);

var AST_ForOf = DEFNODE("ForOf", "await", function AST_ForOf(props) {
    if (props) {
        this.await = props.await;
        this.init = props.init;
        this.object = props.object;
        this.block_scope = props.block_scope;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `for ... of` statement",
}, AST_ForIn);

var AST_With = DEFNODE("With", "expression", function AST_With(props) {
    if (props) {
        this.expression = props.expression;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `with` statement",
    $propdoc: {
        expression: "[AST_Node] the `with` expression"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
            this.body._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.body);
        push(this.expression);
    },
}, AST_StatementWithBody);

/* -----[ scope and functions ]----- */

var AST_Scope = DEFNODE(
    "Scope",
    "variables uses_with uses_eval parent_scope enclosed cname",
    function AST_Scope(props) {
        if (props) {
            this.variables = props.variables;
            this.uses_with = props.uses_with;
            this.uses_eval = props.uses_eval;
            this.parent_scope = props.parent_scope;
            this.enclosed = props.enclosed;
            this.cname = props.cname;
            this.body = props.body;
            this.block_scope = props.block_scope;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "Base class for all statements introducing a lexical scope",
        $propdoc: {
            variables: "[Map/S] a map of name -> SymbolDef for all variables/functions defined in this scope",
            uses_with: "[boolean/S] tells whether this scope uses the `with` statement",
            uses_eval: "[boolean/S] tells whether this scope contains a direct call to the global `eval`",
            parent_scope: "[AST_Scope?/S] link to the parent scope",
            enclosed: "[SymbolDef*/S] a list of all symbol definitions that are accessed from this scope or any subscopes",
            cname: "[integer/S] current index for mangling variables (used internally by the mangler)",
        },
        get_defun_scope: function() {
            var self = this;
            while (self.is_block_scope()) {
                self = self.parent_scope;
            }
            return self;
        },
        clone: function(deep, toplevel) {
            var node = this._clone(deep);
            if (deep && this.variables && toplevel && !this._block_scope) {
                node.figure_out_scope({}, {
                    toplevel: toplevel,
                    parent_scope: this.parent_scope
                });
            } else {
                if (this.variables) node.variables = new Map(this.variables);
                if (this.enclosed) node.enclosed = this.enclosed.slice();
                if (this._block_scope) node._block_scope = this._block_scope;
            }
            return node;
        },
        pinned: function() {
            return this.uses_eval || this.uses_with;
        }
    },
    AST_Block
);

var AST_Toplevel = DEFNODE("Toplevel", "globals", function AST_Toplevel(props) {
    if (props) {
        this.globals = props.globals;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The toplevel scope",
    $propdoc: {
        globals: "[Map/S] a map of name -> SymbolDef for all undeclared names",
    },
    wrap_commonjs: function(name) {
        var body = this.body;
        var wrapped_tl = "(function(exports){'$ORIG';})(typeof " + name + "=='undefined'?(" + name + "={}):" + name + ");";
        wrapped_tl = parse(wrapped_tl);
        wrapped_tl = wrapped_tl.transform(new TreeTransformer(function(node) {
            if (node instanceof AST_Directive && node.value == "$ORIG") {
                return MAP.splice(body);
            }
        }));
        return wrapped_tl;
    },
    wrap_enclose: function(args_values) {
        if (typeof args_values != "string") args_values = "";
        var index = args_values.indexOf(":");
        if (index < 0) index = args_values.length;
        var body = this.body;
        return parse([
            "(function(",
            args_values.slice(0, index),
            '){"$ORIG"})(',
            args_values.slice(index + 1),
            ")"
        ].join("")).transform(new TreeTransformer(function(node) {
            if (node instanceof AST_Directive && node.value == "$ORIG") {
                return MAP.splice(body);
            }
        }));
    }
}, AST_Scope);

var AST_Expansion = DEFNODE("Expansion", "expression", function AST_Expansion(props) {
    if (props) {
        this.expression = props.expression;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An expandible argument, such as ...rest, a splat, such as [1,2,...all], or an expansion in a variable declaration, such as var [first, ...rest] = list",
    $propdoc: {
        expression: "[AST_Node] the thing to be expanded"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression.walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.expression);
    },
});

var AST_Lambda = DEFNODE(
    "Lambda",
    "name argnames uses_arguments is_generator async",
    function AST_Lambda(props) {
        if (props) {
            this.name = props.name;
            this.argnames = props.argnames;
            this.uses_arguments = props.uses_arguments;
            this.is_generator = props.is_generator;
            this.async = props.async;
            this.variables = props.variables;
            this.uses_with = props.uses_with;
            this.uses_eval = props.uses_eval;
            this.parent_scope = props.parent_scope;
            this.enclosed = props.enclosed;
            this.cname = props.cname;
            this.body = props.body;
            this.block_scope = props.block_scope;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "Base class for functions",
        $propdoc: {
            name: "[AST_SymbolDeclaration?] the name of this function",
            argnames: "[AST_SymbolFunarg|AST_Destructuring|AST_Expansion|AST_DefaultAssign*] array of function arguments, destructurings, or expanding arguments",
            uses_arguments: "[boolean/S] tells whether this function accesses the arguments array",
            is_generator: "[boolean] is this a generator method",
            async: "[boolean] is this method async",
        },
        args_as_names: function () {
            var out = [];
            for (var i = 0; i < this.argnames.length; i++) {
                if (this.argnames[i] instanceof AST_Destructuring) {
                    out.push(...this.argnames[i].all_symbols());
                } else {
                    out.push(this.argnames[i]);
                }
            }
            return out;
        },
        _walk: function(visitor) {
            return visitor._visit(this, function() {
                if (this.name) this.name._walk(visitor);
                var argnames = this.argnames;
                for (var i = 0, len = argnames.length; i < len; i++) {
                    argnames[i]._walk(visitor);
                }
                walk_body(this, visitor);
            });
        },
        _children_backwards(push) {
            let i = this.body.length;
            while (i--) push(this.body[i]);

            i = this.argnames.length;
            while (i--) push(this.argnames[i]);

            if (this.name) push(this.name);
        },
        is_braceless() {
            return this.body[0] instanceof AST_Return && this.body[0].value;
        },
        // Default args and expansion don't count, so .argnames.length doesn't cut it
        length_property() {
            let length = 0;

            for (const arg of this.argnames) {
                if (arg instanceof AST_SymbolFunarg || arg instanceof AST_Destructuring) {
                    length++;
                }
            }

            return length;
        }
    },
    AST_Scope
);

var AST_Accessor = DEFNODE("Accessor", null, function AST_Accessor(props) {
    if (props) {
        this.name = props.name;
        this.argnames = props.argnames;
        this.uses_arguments = props.uses_arguments;
        this.is_generator = props.is_generator;
        this.async = props.async;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A setter/getter function.  The `name` property is always null."
}, AST_Lambda);

var AST_Function = DEFNODE("Function", null, function AST_Function(props) {
    if (props) {
        this.name = props.name;
        this.argnames = props.argnames;
        this.uses_arguments = props.uses_arguments;
        this.is_generator = props.is_generator;
        this.async = props.async;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A function expression"
}, AST_Lambda);

var AST_Arrow = DEFNODE("Arrow", null, function AST_Arrow(props) {
    if (props) {
        this.name = props.name;
        this.argnames = props.argnames;
        this.uses_arguments = props.uses_arguments;
        this.is_generator = props.is_generator;
        this.async = props.async;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An ES6 Arrow function ((a) => b)"
}, AST_Lambda);

var AST_Defun = DEFNODE("Defun", null, function AST_Defun(props) {
    if (props) {
        this.name = props.name;
        this.argnames = props.argnames;
        this.uses_arguments = props.uses_arguments;
        this.is_generator = props.is_generator;
        this.async = props.async;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A function definition"
}, AST_Lambda);

/* -----[ DESTRUCTURING ]----- */
var AST_Destructuring = DEFNODE("Destructuring", "names is_array", function AST_Destructuring(props) {
    if (props) {
        this.names = props.names;
        this.is_array = props.is_array;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A destructuring of several names. Used in destructuring assignment and with destructuring function argument names",
    $propdoc: {
        "names": "[AST_Node*] Array of properties or elements",
        "is_array": "[Boolean] Whether the destructuring represents an object or array"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.names.forEach(function(name) {
                name._walk(visitor);
            });
        });
    },
    _children_backwards(push) {
        let i = this.names.length;
        while (i--) push(this.names[i]);
    },
    all_symbols: function() {
        var out = [];
        walk(this, node => {
            if (node instanceof AST_SymbolDeclaration) {
                out.push(node);
            }
            if (node instanceof AST_Lambda) {
                return true;
            }
        });
        return out;
    }
});

var AST_PrefixedTemplateString = DEFNODE(
    "PrefixedTemplateString",
    "template_string prefix",
    function AST_PrefixedTemplateString(props) {
        if (props) {
            this.template_string = props.template_string;
            this.prefix = props.prefix;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "A templatestring with a prefix, such as String.raw`foobarbaz`",
        $propdoc: {
            template_string: "[AST_TemplateString] The template string",
            prefix: "[AST_Node] The prefix, which will get called."
        },
        _walk: function(visitor) {
            return visitor._visit(this, function () {
                this.prefix._walk(visitor);
                this.template_string._walk(visitor);
            });
        },
        _children_backwards(push) {
            push(this.template_string);
            push(this.prefix);
        },
    }
);

var AST_TemplateString = DEFNODE("TemplateString", "segments", function AST_TemplateString(props) {
    if (props) {
        this.segments = props.segments;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A template string literal",
    $propdoc: {
        segments: "[AST_Node*] One or more segments, starting with AST_TemplateSegment. AST_Node may follow AST_TemplateSegment, but each AST_Node must be followed by AST_TemplateSegment."
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.segments.forEach(function(seg) {
                seg._walk(visitor);
            });
        });
    },
    _children_backwards(push) {
        let i = this.segments.length;
        while (i--) push(this.segments[i]);
    }
});

var AST_TemplateSegment = DEFNODE("TemplateSegment", "value raw", function AST_TemplateSegment(props) {
    if (props) {
        this.value = props.value;
        this.raw = props.raw;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A segment of a template string literal",
    $propdoc: {
        value: "Content of the segment",
        raw: "Raw source of the segment",
    }
});

/* -----[ JUMPS ]----- */

var AST_Jump = DEFNODE("Jump", null, function AST_Jump(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for “jumps” (for now that's `return`, `throw`, `break` and `continue`)"
}, AST_Statement);

/** Base class for “exits” (`return` and `throw`) */
var AST_Exit = DEFNODE("Exit", "value", function AST_Exit(props) {
    if (props) {
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for “exits” (`return` and `throw`)",
    $propdoc: {
        value: "[AST_Node?] the value returned or thrown by this statement; could be null for AST_Return"
    },
    _walk: function(visitor) {
        return visitor._visit(this, this.value && function() {
            this.value._walk(visitor);
        });
    },
    _children_backwards(push) {
        if (this.value) push(this.value);
    },
}, AST_Jump);

var AST_Return = DEFNODE("Return", null, function AST_Return(props) {
    if (props) {
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `return` statement"
}, AST_Exit);

var AST_Throw = DEFNODE("Throw", null, function AST_Throw(props) {
    if (props) {
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `throw` statement"
}, AST_Exit);

var AST_LoopControl = DEFNODE("LoopControl", "label", function AST_LoopControl(props) {
    if (props) {
        this.label = props.label;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for loop control statements (`break` and `continue`)",
    $propdoc: {
        label: "[AST_LabelRef?] the label, or null if none",
    },
    _walk: function(visitor) {
        return visitor._visit(this, this.label && function() {
            this.label._walk(visitor);
        });
    },
    _children_backwards(push) {
        if (this.label) push(this.label);
    },
}, AST_Jump);

var AST_Break = DEFNODE("Break", null, function AST_Break(props) {
    if (props) {
        this.label = props.label;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `break` statement"
}, AST_LoopControl);

var AST_Continue = DEFNODE("Continue", null, function AST_Continue(props) {
    if (props) {
        this.label = props.label;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `continue` statement"
}, AST_LoopControl);

var AST_Await = DEFNODE("Await", "expression", function AST_Await(props) {
    if (props) {
        this.expression = props.expression;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An `await` statement",
    $propdoc: {
        expression: "[AST_Node] the mandatory expression being awaited",
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.expression);
    },
});

var AST_Yield = DEFNODE("Yield", "expression is_star", function AST_Yield(props) {
    if (props) {
        this.expression = props.expression;
        this.is_star = props.is_star;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `yield` statement",
    $propdoc: {
        expression: "[AST_Node?] the value returned or thrown by this statement; could be null (representing undefined) but only when is_star is set to false",
        is_star: "[Boolean] Whether this is a yield or yield* statement"
    },
    _walk: function(visitor) {
        return visitor._visit(this, this.expression && function() {
            this.expression._walk(visitor);
        });
    },
    _children_backwards(push) {
        if (this.expression) push(this.expression);
    }
});

/* -----[ IF ]----- */

var AST_If = DEFNODE("If", "condition alternative", function AST_If(props) {
    if (props) {
        this.condition = props.condition;
        this.alternative = props.alternative;
        this.body = props.body;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `if` statement",
    $propdoc: {
        condition: "[AST_Node] the `if` condition",
        alternative: "[AST_Statement?] the `else` part, or null if not present"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.condition._walk(visitor);
            this.body._walk(visitor);
            if (this.alternative) this.alternative._walk(visitor);
        });
    },
    _children_backwards(push) {
        if (this.alternative) {
            push(this.alternative);
        }
        push(this.body);
        push(this.condition);
    }
}, AST_StatementWithBody);

/* -----[ SWITCH ]----- */

var AST_Switch = DEFNODE("Switch", "expression", function AST_Switch(props) {
    if (props) {
        this.expression = props.expression;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `switch` statement",
    $propdoc: {
        expression: "[AST_Node] the `switch` “discriminant”"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
            walk_body(this, visitor);
        });
    },
    _children_backwards(push) {
        let i = this.body.length;
        while (i--) push(this.body[i]);
        push(this.expression);
    }
}, AST_Block);

var AST_SwitchBranch = DEFNODE("SwitchBranch", null, function AST_SwitchBranch(props) {
    if (props) {
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for `switch` branches",
}, AST_Block);

var AST_Default = DEFNODE("Default", null, function AST_Default(props) {
    if (props) {
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `default` switch branch",
}, AST_SwitchBranch);

var AST_Case = DEFNODE("Case", "expression", function AST_Case(props) {
    if (props) {
        this.expression = props.expression;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `case` switch branch",
    $propdoc: {
        expression: "[AST_Node] the `case` expression"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
            walk_body(this, visitor);
        });
    },
    _children_backwards(push) {
        let i = this.body.length;
        while (i--) push(this.body[i]);
        push(this.expression);
    },
}, AST_SwitchBranch);

/* -----[ EXCEPTIONS ]----- */

var AST_Try = DEFNODE("Try", "body bcatch bfinally", function AST_Try(props) {
    if (props) {
        this.body = props.body;
        this.bcatch = props.bcatch;
        this.bfinally = props.bfinally;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `try` statement",
    $propdoc: {
        body: "[AST_TryBlock] the try block",
        bcatch: "[AST_Catch?] the catch block, or null if not present",
        bfinally: "[AST_Finally?] the finally block, or null if not present"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.body._walk(visitor);
            if (this.bcatch) this.bcatch._walk(visitor);
            if (this.bfinally) this.bfinally._walk(visitor);
        });
    },
    _children_backwards(push) {
        if (this.bfinally) push(this.bfinally);
        if (this.bcatch) push(this.bcatch);
        push(this.body);
    },
}, AST_Statement);

var AST_TryBlock = DEFNODE("TryBlock", null, function AST_TryBlock(props) {
    if (props) {
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `try` block of a try statement"
}, AST_Block);

var AST_Catch = DEFNODE("Catch", "argname", function AST_Catch(props) {
    if (props) {
        this.argname = props.argname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `catch` node; only makes sense as part of a `try` statement",
    $propdoc: {
        argname: "[AST_SymbolCatch|AST_Destructuring|AST_Expansion|AST_DefaultAssign] symbol for the exception"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            if (this.argname) this.argname._walk(visitor);
            walk_body(this, visitor);
        });
    },
    _children_backwards(push) {
        let i = this.body.length;
        while (i--) push(this.body[i]);
        if (this.argname) push(this.argname);
    },
}, AST_Block);

var AST_Finally = DEFNODE("Finally", null, function AST_Finally(props) {
    if (props) {
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `finally` node; only makes sense as part of a `try` statement"
}, AST_Block);

/* -----[ VAR/CONST ]----- */

var AST_Definitions = DEFNODE("Definitions", "definitions", function AST_Definitions(props) {
    if (props) {
        this.definitions = props.definitions;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for `var` or `const` nodes (variable declarations/initializations)",
    $propdoc: {
        definitions: "[AST_VarDef*] array of variable definitions"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            var definitions = this.definitions;
            for (var i = 0, len = definitions.length; i < len; i++) {
                definitions[i]._walk(visitor);
            }
        });
    },
    _children_backwards(push) {
        let i = this.definitions.length;
        while (i--) push(this.definitions[i]);
    },
}, AST_Statement);

var AST_Var = DEFNODE("Var", null, function AST_Var(props) {
    if (props) {
        this.definitions = props.definitions;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `var` statement"
}, AST_Definitions);

var AST_Let = DEFNODE("Let", null, function AST_Let(props) {
    if (props) {
        this.definitions = props.definitions;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `let` statement"
}, AST_Definitions);

var AST_Const = DEFNODE("Const", null, function AST_Const(props) {
    if (props) {
        this.definitions = props.definitions;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A `const` statement"
}, AST_Definitions);

var AST_VarDef = DEFNODE("VarDef", "name value", function AST_VarDef(props) {
    if (props) {
        this.name = props.name;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A variable declaration; only appears in a AST_Definitions node",
    $propdoc: {
        name: "[AST_Destructuring|AST_SymbolConst|AST_SymbolLet|AST_SymbolVar] name of the variable",
        value: "[AST_Node?] initializer, or null of there's no initializer"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.name._walk(visitor);
            if (this.value) this.value._walk(visitor);
        });
    },
    _children_backwards(push) {
        if (this.value) push(this.value);
        push(this.name);
    },
    declarations_as_names() {
        if (this.name instanceof AST_SymbolDeclaration) {
            return [this];
        } else {
            return this.name.all_symbols();
        }
    }
});

var AST_NameMapping = DEFNODE("NameMapping", "foreign_name name", function AST_NameMapping(props) {
    if (props) {
        this.foreign_name = props.foreign_name;
        this.name = props.name;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The part of the export/import statement that declare names from a module.",
    $propdoc: {
        foreign_name: "[AST_SymbolExportForeign|AST_SymbolImportForeign] The name being exported/imported (as specified in the module)",
        name: "[AST_SymbolExport|AST_SymbolImport] The name as it is visible to this module."
    },
    _walk: function (visitor) {
        return visitor._visit(this, function() {
            this.foreign_name._walk(visitor);
            this.name._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.name);
        push(this.foreign_name);
    },
});

var AST_Import = DEFNODE(
    "Import",
    "imported_name imported_names module_name assert_clause",
    function AST_Import(props) {
        if (props) {
            this.imported_name = props.imported_name;
            this.imported_names = props.imported_names;
            this.module_name = props.module_name;
            this.assert_clause = props.assert_clause;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "An `import` statement",
        $propdoc: {
            imported_name: "[AST_SymbolImport] The name of the variable holding the module's default export.",
            imported_names: "[AST_NameMapping*] The names of non-default imported variables",
            module_name: "[AST_String] String literal describing where this module came from",
            assert_clause: "[AST_Object?] The import assertion"
        },
        _walk: function(visitor) {
            return visitor._visit(this, function() {
                if (this.imported_name) {
                    this.imported_name._walk(visitor);
                }
                if (this.imported_names) {
                    this.imported_names.forEach(function(name_import) {
                        name_import._walk(visitor);
                    });
                }
                this.module_name._walk(visitor);
            });
        },
        _children_backwards(push) {
            push(this.module_name);
            if (this.imported_names) {
                let i = this.imported_names.length;
                while (i--) push(this.imported_names[i]);
            }
            if (this.imported_name) push(this.imported_name);
        },
    }
);

var AST_ImportMeta = DEFNODE("ImportMeta", null, function AST_ImportMeta(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A reference to import.meta",
});

var AST_Export = DEFNODE(
    "Export",
    "exported_definition exported_value is_default exported_names module_name assert_clause",
    function AST_Export(props) {
        if (props) {
            this.exported_definition = props.exported_definition;
            this.exported_value = props.exported_value;
            this.is_default = props.is_default;
            this.exported_names = props.exported_names;
            this.module_name = props.module_name;
            this.assert_clause = props.assert_clause;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "An `export` statement",
        $propdoc: {
            exported_definition: "[AST_Defun|AST_Definitions|AST_DefClass?] An exported definition",
            exported_value: "[AST_Node?] An exported value",
            exported_names: "[AST_NameMapping*?] List of exported names",
            module_name: "[AST_String?] Name of the file to load exports from",
            is_default: "[Boolean] Whether this is the default exported value of this module",
            assert_clause: "[AST_Object?] The import assertion"
        },
        _walk: function (visitor) {
            return visitor._visit(this, function () {
                if (this.exported_definition) {
                    this.exported_definition._walk(visitor);
                }
                if (this.exported_value) {
                    this.exported_value._walk(visitor);
                }
                if (this.exported_names) {
                    this.exported_names.forEach(function(name_export) {
                        name_export._walk(visitor);
                    });
                }
                if (this.module_name) {
                    this.module_name._walk(visitor);
                }
            });
        },
        _children_backwards(push) {
            if (this.module_name) push(this.module_name);
            if (this.exported_names) {
                let i = this.exported_names.length;
                while (i--) push(this.exported_names[i]);
            }
            if (this.exported_value) push(this.exported_value);
            if (this.exported_definition) push(this.exported_definition);
        }
    },
    AST_Statement
);

/* -----[ OTHER ]----- */

var AST_Call = DEFNODE(
    "Call",
    "expression args optional _annotations",
    function AST_Call(props) {
        if (props) {
            this.expression = props.expression;
            this.args = props.args;
            this.optional = props.optional;
            this._annotations = props._annotations;
            this.start = props.start;
            this.end = props.end;
            this.initialize();
        }

        this.flags = 0;
    },
    {
        $documentation: "A function call expression",
        $propdoc: {
            expression: "[AST_Node] expression to invoke as function",
            args: "[AST_Node*] array of arguments",
            optional: "[boolean] whether this is an optional call (IE ?.() )",
            _annotations: "[number] bitfield containing information about the call"
        },
        initialize() {
            if (this._annotations == null) this._annotations = 0;
        },
        _walk(visitor) {
            return visitor._visit(this, function() {
                var args = this.args;
                for (var i = 0, len = args.length; i < len; i++) {
                    args[i]._walk(visitor);
                }
                this.expression._walk(visitor);  // TODO why do we need to crawl this last?
            });
        },
        _children_backwards(push) {
            let i = this.args.length;
            while (i--) push(this.args[i]);
            push(this.expression);
        },
    }
);

var AST_New = DEFNODE("New", null, function AST_New(props) {
    if (props) {
        this.expression = props.expression;
        this.args = props.args;
        this.optional = props.optional;
        this._annotations = props._annotations;
        this.start = props.start;
        this.end = props.end;
        this.initialize();
    }

    this.flags = 0;
}, {
    $documentation: "An object instantiation.  Derives from a function call since it has exactly the same properties"
}, AST_Call);

var AST_Sequence = DEFNODE("Sequence", "expressions", function AST_Sequence(props) {
    if (props) {
        this.expressions = props.expressions;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A sequence expression (comma-separated expressions)",
    $propdoc: {
        expressions: "[AST_Node*] array of expressions (at least two)"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expressions.forEach(function(node) {
                node._walk(visitor);
            });
        });
    },
    _children_backwards(push) {
        let i = this.expressions.length;
        while (i--) push(this.expressions[i]);
    },
});

var AST_PropAccess = DEFNODE(
    "PropAccess",
    "expression property optional",
    function AST_PropAccess(props) {
        if (props) {
            this.expression = props.expression;
            this.property = props.property;
            this.optional = props.optional;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "Base class for property access expressions, i.e. `a.foo` or `a[\"foo\"]`",
        $propdoc: {
            expression: "[AST_Node] the “container” expression",
            property: "[AST_Node|string] the property to access.  For AST_Dot & AST_DotHash this is always a plain string, while for AST_Sub it's an arbitrary AST_Node",

            optional: "[boolean] whether this is an optional property access (IE ?.)"
        }
    }
);

var AST_Dot = DEFNODE("Dot", "quote", function AST_Dot(props) {
    if (props) {
        this.quote = props.quote;
        this.expression = props.expression;
        this.property = props.property;
        this.optional = props.optional;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A dotted property access expression",
    $propdoc: {
        quote: "[string] the original quote character when transformed from AST_Sub",
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.expression);
    },
}, AST_PropAccess);

var AST_DotHash = DEFNODE("DotHash", "", function AST_DotHash(props) {
    if (props) {
        this.expression = props.expression;
        this.property = props.property;
        this.optional = props.optional;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A dotted property access to a private property",
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.expression);
    },
}, AST_PropAccess);

var AST_Sub = DEFNODE("Sub", null, function AST_Sub(props) {
    if (props) {
        this.expression = props.expression;
        this.property = props.property;
        this.optional = props.optional;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Index-style property access, i.e. `a[\"foo\"]`",
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
            this.property._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.property);
        push(this.expression);
    },
}, AST_PropAccess);

var AST_Chain = DEFNODE("Chain", "expression", function AST_Chain(props) {
    if (props) {
        this.expression = props.expression;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A chain expression like a?.b?.(c)?.[d]",
    $propdoc: {
        expression: "[AST_Call|AST_Dot|AST_DotHash|AST_Sub] chain element."
    },
    _walk: function (visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.expression);
    },
});

var AST_Unary = DEFNODE("Unary", "operator expression", function AST_Unary(props) {
    if (props) {
        this.operator = props.operator;
        this.expression = props.expression;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for unary expressions",
    $propdoc: {
        operator: "[string] the operator",
        expression: "[AST_Node] expression that this unary operator applies to"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.expression._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.expression);
    },
});

var AST_UnaryPrefix = DEFNODE("UnaryPrefix", null, function AST_UnaryPrefix(props) {
    if (props) {
        this.operator = props.operator;
        this.expression = props.expression;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Unary prefix expression, i.e. `typeof i` or `++i`"
}, AST_Unary);

var AST_UnaryPostfix = DEFNODE("UnaryPostfix", null, function AST_UnaryPostfix(props) {
    if (props) {
        this.operator = props.operator;
        this.expression = props.expression;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Unary postfix expression, i.e. `i++`"
}, AST_Unary);

var AST_Binary = DEFNODE("Binary", "operator left right", function AST_Binary(props) {
    if (props) {
        this.operator = props.operator;
        this.left = props.left;
        this.right = props.right;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Binary expression, i.e. `a + b`",
    $propdoc: {
        left: "[AST_Node] left-hand side expression",
        operator: "[string] the operator",
        right: "[AST_Node] right-hand side expression"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.left._walk(visitor);
            this.right._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.right);
        push(this.left);
    },
});

var AST_Conditional = DEFNODE(
    "Conditional",
    "condition consequent alternative",
    function AST_Conditional(props) {
        if (props) {
            this.condition = props.condition;
            this.consequent = props.consequent;
            this.alternative = props.alternative;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "Conditional expression using the ternary operator, i.e. `a ? b : c`",
        $propdoc: {
            condition: "[AST_Node]",
            consequent: "[AST_Node]",
            alternative: "[AST_Node]"
        },
        _walk: function(visitor) {
            return visitor._visit(this, function() {
                this.condition._walk(visitor);
                this.consequent._walk(visitor);
                this.alternative._walk(visitor);
            });
        },
        _children_backwards(push) {
            push(this.alternative);
            push(this.consequent);
            push(this.condition);
        },
    }
);

var AST_Assign = DEFNODE("Assign", "logical", function AST_Assign(props) {
    if (props) {
        this.logical = props.logical;
        this.operator = props.operator;
        this.left = props.left;
        this.right = props.right;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An assignment expression — `a = b + 5`",
    $propdoc: {
        logical: "Whether it's a logical assignment"
    }
}, AST_Binary);

var AST_DefaultAssign = DEFNODE("DefaultAssign", null, function AST_DefaultAssign(props) {
    if (props) {
        this.operator = props.operator;
        this.left = props.left;
        this.right = props.right;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A default assignment expression like in `(a = 3) => a`"
}, AST_Binary);

/* -----[ LITERALS ]----- */

var AST_Array = DEFNODE("Array", "elements", function AST_Array(props) {
    if (props) {
        this.elements = props.elements;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An array literal",
    $propdoc: {
        elements: "[AST_Node*] array of elements"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            var elements = this.elements;
            for (var i = 0, len = elements.length; i < len; i++) {
                elements[i]._walk(visitor);
            }
        });
    },
    _children_backwards(push) {
        let i = this.elements.length;
        while (i--) push(this.elements[i]);
    },
});

var AST_Object = DEFNODE("Object", "properties", function AST_Object(props) {
    if (props) {
        this.properties = props.properties;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An object literal",
    $propdoc: {
        properties: "[AST_ObjectProperty*] array of properties"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            var properties = this.properties;
            for (var i = 0, len = properties.length; i < len; i++) {
                properties[i]._walk(visitor);
            }
        });
    },
    _children_backwards(push) {
        let i = this.properties.length;
        while (i--) push(this.properties[i]);
    },
});

var AST_ObjectProperty = DEFNODE("ObjectProperty", "key value", function AST_ObjectProperty(props) {
    if (props) {
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
        this._annotations = props._annotations;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for literal object properties",
    $propdoc: {
        key: "[string|AST_Node] property name. For ObjectKeyVal this is a string. For getters, setters and computed property this is an AST_Node.",
        value: "[AST_Node] property value.  For getters and setters this is an AST_Accessor."
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            if (this.key instanceof AST_Node)
                this.key._walk(visitor);
            this.value._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.value);
        if (this.key instanceof AST_Node) push(this.key);
    }
});

var AST_ObjectKeyVal = DEFNODE("ObjectKeyVal", "quote", function AST_ObjectKeyVal(props) {
    if (props) {
        this.quote = props.quote;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
        this._annotations = props._annotations;
    }

    this.flags = 0;
}, {
    $documentation: "A key: value object property",
    $propdoc: {
        quote: "[string] the original quote character"
    },
    computed_key() {
        return this.key instanceof AST_Node;
    }
}, AST_ObjectProperty);

var AST_PrivateSetter = DEFNODE("PrivateSetter", "static", function AST_PrivateSetter(props) {
    if (props) {
        this.static = props.static;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $propdoc: {
        static: "[boolean] whether this is a static private setter"
    },
    $documentation: "A private setter property",
    computed_key() {
        return false;
    }
}, AST_ObjectProperty);

var AST_PrivateGetter = DEFNODE("PrivateGetter", "static", function AST_PrivateGetter(props) {
    if (props) {
        this.static = props.static;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $propdoc: {
        static: "[boolean] whether this is a static private getter"
    },
    $documentation: "A private getter property",
    computed_key() {
        return false;
    }
}, AST_ObjectProperty);

var AST_ObjectSetter = DEFNODE("ObjectSetter", "quote static", function AST_ObjectSetter(props) {
    if (props) {
        this.quote = props.quote;
        this.static = props.static;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
        this._annotations = props._annotations;
    }

    this.flags = 0;
}, {
    $propdoc: {
        quote: "[string|undefined] the original quote character, if any",
        static: "[boolean] whether this is a static setter (classes only)"
    },
    $documentation: "An object setter property",
    computed_key() {
        return !(this.key instanceof AST_SymbolMethod);
    }
}, AST_ObjectProperty);

var AST_ObjectGetter = DEFNODE("ObjectGetter", "quote static", function AST_ObjectGetter(props) {
    if (props) {
        this.quote = props.quote;
        this.static = props.static;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
        this._annotations = props._annotations;
    }

    this.flags = 0;
}, {
    $propdoc: {
        quote: "[string|undefined] the original quote character, if any",
        static: "[boolean] whether this is a static getter (classes only)"
    },
    $documentation: "An object getter property",
    computed_key() {
        return !(this.key instanceof AST_SymbolMethod);
    }
}, AST_ObjectProperty);

var AST_ConciseMethod = DEFNODE(
    "ConciseMethod",
    "quote static is_generator async",
    function AST_ConciseMethod(props) {
        if (props) {
            this.quote = props.quote;
            this.static = props.static;
            this.is_generator = props.is_generator;
            this.async = props.async;
            this.key = props.key;
            this.value = props.value;
            this.start = props.start;
            this.end = props.end;
            this._annotations = props._annotations;
        }

        this.flags = 0;
    },
    {
        $propdoc: {
            quote: "[string|undefined] the original quote character, if any",
            static: "[boolean] is this method static (classes only)",
            is_generator: "[boolean] is this a generator method",
            async: "[boolean] is this method async",
        },
        $documentation: "An ES6 concise method inside an object or class",
        computed_key() {
            return !(this.key instanceof AST_SymbolMethod);
        }
    },
    AST_ObjectProperty
);

var AST_PrivateMethod = DEFNODE("PrivateMethod", "", function AST_PrivateMethod(props) {
    if (props) {
        this.quote = props.quote;
        this.static = props.static;
        this.is_generator = props.is_generator;
        this.async = props.async;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A private class method inside a class",
}, AST_ConciseMethod);

var AST_Class = DEFNODE("Class", "name extends properties", function AST_Class(props) {
    if (props) {
        this.name = props.name;
        this.extends = props.extends;
        this.properties = props.properties;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $propdoc: {
        name: "[AST_SymbolClass|AST_SymbolDefClass?] optional class name.",
        extends: "[AST_Node]? optional parent class",
        properties: "[AST_ObjectProperty*] array of properties"
    },
    $documentation: "An ES6 class",
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            if (this.name) {
                this.name._walk(visitor);
            }
            if (this.extends) {
                this.extends._walk(visitor);
            }
            this.properties.forEach((prop) => prop._walk(visitor));
        });
    },
    _children_backwards(push) {
        let i = this.properties.length;
        while (i--) push(this.properties[i]);
        if (this.extends) push(this.extends);
        if (this.name) push(this.name);
    },
    /** go through the bits that are executed instantly, not when the class is `new`'d. Doesn't walk the name. */
    visit_nondeferred_class_parts(visitor) {
        if (this.extends) {
            this.extends._walk(visitor);
        }
        this.properties.forEach((prop) => {
            if (prop instanceof AST_ClassStaticBlock) {
                prop._walk(visitor);
                return;
            }
            if (prop.computed_key()) {
                visitor.push(prop);
                prop.key._walk(visitor);
                visitor.pop();
            }
            if ((prop instanceof AST_ClassPrivateProperty || prop instanceof AST_ClassProperty) && prop.static && prop.value) {
                visitor.push(prop);
                prop.value._walk(visitor);
                visitor.pop();
            }
        });
    },
    /** go through the bits that are executed later, when the class is `new`'d or a static method is called */
    visit_deferred_class_parts(visitor) {
        this.properties.forEach((prop) => {
            if (prop instanceof AST_ConciseMethod) {
                prop.walk(visitor);
            } else if (prop instanceof AST_ClassProperty && !prop.static && prop.value) {
                visitor.push(prop);
                prop.value._walk(visitor);
                visitor.pop();
            }
        });
    },
}, AST_Scope /* TODO a class might have a scope but it's not a scope */);

var AST_ClassProperty = DEFNODE("ClassProperty", "static quote", function AST_ClassProperty(props) {
    if (props) {
        this.static = props.static;
        this.quote = props.quote;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
        this._annotations = props._annotations;
    }

    this.flags = 0;
}, {
    $documentation: "A class property",
    $propdoc: {
        static: "[boolean] whether this is a static key",
        quote: "[string] which quote is being used"
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            if (this.key instanceof AST_Node)
                this.key._walk(visitor);
            if (this.value instanceof AST_Node)
                this.value._walk(visitor);
        });
    },
    _children_backwards(push) {
        if (this.value instanceof AST_Node) push(this.value);
        if (this.key instanceof AST_Node) push(this.key);
    },
    computed_key() {
        return !(this.key instanceof AST_SymbolClassProperty);
    }
}, AST_ObjectProperty);

var AST_ClassPrivateProperty = DEFNODE("ClassPrivateProperty", "", function AST_ClassPrivateProperty(props) {
    if (props) {
        this.static = props.static;
        this.quote = props.quote;
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A class property for a private property",
}, AST_ClassProperty);

var AST_PrivateIn = DEFNODE("PrivateIn", "key value", function AST_PrivateIn(props) {
    if (props) {
        this.key = props.key;
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "An `in` binop when the key is private, eg #x in this",
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            this.key._walk(visitor);
            this.value._walk(visitor);
        });
    },
    _children_backwards(push) {
        push(this.value);
        push(this.key);
    },
});

var AST_DefClass = DEFNODE("DefClass", null, function AST_DefClass(props) {
    if (props) {
        this.name = props.name;
        this.extends = props.extends;
        this.properties = props.properties;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A class definition",
}, AST_Class);

var AST_ClassStaticBlock = DEFNODE("ClassStaticBlock", "body block_scope", function AST_ClassStaticBlock (props) {
    this.body = props.body;
    this.block_scope = props.block_scope;
    this.start = props.start;
    this.end = props.end;
}, {
    $documentation: "A block containing statements to be executed in the context of the class",
    $propdoc: {
        body: "[AST_Statement*] an array of statements",
    },
    _walk: function(visitor) {
        return visitor._visit(this, function() {
            walk_body(this, visitor);
        });
    },
    _children_backwards(push) {
        let i = this.body.length;
        while (i--) push(this.body[i]);
    },
    clone: clone_block_scope,
    computed_key: () => false
}, AST_Scope);

var AST_ClassExpression = DEFNODE("ClassExpression", null, function AST_ClassExpression(props) {
    if (props) {
        this.name = props.name;
        this.extends = props.extends;
        this.properties = props.properties;
        this.variables = props.variables;
        this.uses_with = props.uses_with;
        this.uses_eval = props.uses_eval;
        this.parent_scope = props.parent_scope;
        this.enclosed = props.enclosed;
        this.cname = props.cname;
        this.body = props.body;
        this.block_scope = props.block_scope;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A class expression."
}, AST_Class);

var AST_Symbol = DEFNODE("Symbol", "scope name thedef", function AST_Symbol(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $propdoc: {
        name: "[string] name of this symbol",
        scope: "[AST_Scope/S] the current scope (not necessarily the definition scope)",
        thedef: "[SymbolDef/S] the definition of this symbol"
    },
    $documentation: "Base class for all symbols"
});

var AST_NewTarget = DEFNODE("NewTarget", null, function AST_NewTarget(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A reference to new.target"
});

var AST_SymbolDeclaration = DEFNODE("SymbolDeclaration", "init", function AST_SymbolDeclaration(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A declaration symbol (symbol in var/const, function name or argument, symbol in catch)",
}, AST_Symbol);

var AST_SymbolVar = DEFNODE("SymbolVar", null, function AST_SymbolVar(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol defining a variable",
}, AST_SymbolDeclaration);

var AST_SymbolBlockDeclaration = DEFNODE(
    "SymbolBlockDeclaration",
    null,
    function AST_SymbolBlockDeclaration(props) {
        if (props) {
            this.init = props.init;
            this.scope = props.scope;
            this.name = props.name;
            this.thedef = props.thedef;
            this.start = props.start;
            this.end = props.end;
        }

        this.flags = 0;
    },
    {
        $documentation: "Base class for block-scoped declaration symbols"
    },
    AST_SymbolDeclaration
);

var AST_SymbolConst = DEFNODE("SymbolConst", null, function AST_SymbolConst(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A constant declaration"
}, AST_SymbolBlockDeclaration);

var AST_SymbolLet = DEFNODE("SymbolLet", null, function AST_SymbolLet(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A block-scoped `let` declaration"
}, AST_SymbolBlockDeclaration);

var AST_SymbolFunarg = DEFNODE("SymbolFunarg", null, function AST_SymbolFunarg(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol naming a function argument",
}, AST_SymbolVar);

var AST_SymbolDefun = DEFNODE("SymbolDefun", null, function AST_SymbolDefun(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol defining a function",
}, AST_SymbolDeclaration);

var AST_SymbolMethod = DEFNODE("SymbolMethod", null, function AST_SymbolMethod(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol in an object defining a method",
}, AST_Symbol);

var AST_SymbolClassProperty = DEFNODE("SymbolClassProperty", null, function AST_SymbolClassProperty(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol for a class property",
}, AST_Symbol);

var AST_SymbolLambda = DEFNODE("SymbolLambda", null, function AST_SymbolLambda(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol naming a function expression",
}, AST_SymbolDeclaration);

var AST_SymbolDefClass = DEFNODE("SymbolDefClass", null, function AST_SymbolDefClass(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol naming a class's name in a class declaration. Lexically scoped to its containing scope, and accessible within the class."
}, AST_SymbolBlockDeclaration);

var AST_SymbolClass = DEFNODE("SymbolClass", null, function AST_SymbolClass(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol naming a class's name. Lexically scoped to the class."
}, AST_SymbolDeclaration);

var AST_SymbolCatch = DEFNODE("SymbolCatch", null, function AST_SymbolCatch(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol naming the exception in catch",
}, AST_SymbolBlockDeclaration);

var AST_SymbolImport = DEFNODE("SymbolImport", null, function AST_SymbolImport(props) {
    if (props) {
        this.init = props.init;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol referring to an imported name",
}, AST_SymbolBlockDeclaration);

var AST_SymbolImportForeign = DEFNODE("SymbolImportForeign", null, function AST_SymbolImportForeign(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.quote = props.quote;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A symbol imported from a module, but it is defined in the other module, and its real name is irrelevant for this module's purposes",
}, AST_Symbol);

var AST_Label = DEFNODE("Label", "references", function AST_Label(props) {
    if (props) {
        this.references = props.references;
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
        this.initialize();
    }

    this.flags = 0;
}, {
    $documentation: "Symbol naming a label (declaration)",
    $propdoc: {
        references: "[AST_LoopControl*] a list of nodes referring to this label"
    },
    initialize: function() {
        this.references = [];
        this.thedef = this;
    }
}, AST_Symbol);

var AST_SymbolRef = DEFNODE("SymbolRef", null, function AST_SymbolRef(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Reference to some symbol (not definition/declaration)",
}, AST_Symbol);

var AST_SymbolExport = DEFNODE("SymbolExport", null, function AST_SymbolExport(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.quote = props.quote;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Symbol referring to a name to export",
}, AST_SymbolRef);

var AST_SymbolExportForeign = DEFNODE("SymbolExportForeign", null, function AST_SymbolExportForeign(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.quote = props.quote;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A symbol exported from this module, but it is used in the other module, and its real name is irrelevant for this module's purposes",
}, AST_Symbol);

var AST_LabelRef = DEFNODE("LabelRef", null, function AST_LabelRef(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Reference to a label symbol",
}, AST_Symbol);

var AST_SymbolPrivateProperty = DEFNODE("SymbolPrivateProperty", null, function AST_SymbolPrivateProperty(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A symbol that refers to a private property",
}, AST_Symbol);

var AST_This = DEFNODE("This", null, function AST_This(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `this` symbol",
}, AST_Symbol);

var AST_Super = DEFNODE("Super", null, function AST_Super(props) {
    if (props) {
        this.scope = props.scope;
        this.name = props.name;
        this.thedef = props.thedef;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `super` symbol",
}, AST_This);

var AST_Constant = DEFNODE("Constant", null, function AST_Constant(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for all constants",
    getValue: function() {
        return this.value;
    }
});

var AST_String = DEFNODE("String", "value quote", function AST_String(props) {
    if (props) {
        this.value = props.value;
        this.quote = props.quote;
        this.start = props.start;
        this.end = props.end;
        this._annotations = props._annotations;
    }

    this.flags = 0;
}, {
    $documentation: "A string literal",
    $propdoc: {
        value: "[string] the contents of this string",
        quote: "[string] the original quote character"
    }
}, AST_Constant);

var AST_Number = DEFNODE("Number", "value raw", function AST_Number(props) {
    if (props) {
        this.value = props.value;
        this.raw = props.raw;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A number literal",
    $propdoc: {
        value: "[number] the numeric value",
        raw: "[string] numeric value as string"
    }
}, AST_Constant);

var AST_BigInt = DEFNODE("BigInt", "value", function AST_BigInt(props) {
    if (props) {
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A big int literal",
    $propdoc: {
        value: "[string] big int value"
    }
}, AST_Constant);

var AST_RegExp = DEFNODE("RegExp", "value", function AST_RegExp(props) {
    if (props) {
        this.value = props.value;
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A regexp literal",
    $propdoc: {
        value: "[RegExp] the actual regexp",
    }
}, AST_Constant);

var AST_Atom = DEFNODE("Atom", null, function AST_Atom(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for atoms",
}, AST_Constant);

var AST_Null = DEFNODE("Null", null, function AST_Null(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `null` atom",
    value: null
}, AST_Atom);

var AST_NaN = DEFNODE("NaN", null, function AST_NaN(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The impossible value",
    value: 0/0
}, AST_Atom);

var AST_Undefined = DEFNODE("Undefined", null, function AST_Undefined(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `undefined` value",
    value: (function() {}())
}, AST_Atom);

var AST_Hole = DEFNODE("Hole", null, function AST_Hole(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "A hole in an array",
    value: (function() {}())
}, AST_Atom);

var AST_Infinity = DEFNODE("Infinity", null, function AST_Infinity(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `Infinity` value",
    value: 1/0
}, AST_Atom);

var AST_Boolean = DEFNODE("Boolean", null, function AST_Boolean(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "Base class for booleans",
}, AST_Atom);

var AST_False = DEFNODE("False", null, function AST_False(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `false` atom",
    value: false
}, AST_Boolean);

var AST_True = DEFNODE("True", null, function AST_True(props) {
    if (props) {
        this.start = props.start;
        this.end = props.end;
    }

    this.flags = 0;
}, {
    $documentation: "The `true` atom",
    value: true
}, AST_Boolean);

/* -----[ Walk function ]---- */

/**
 * Walk nodes in depth-first search fashion.
 * Callback can return `walk_abort` symbol to stop iteration.
 * It can also return `true` to stop iteration just for child nodes.
 * Iteration can be stopped and continued by passing the `to_visit` argument,
 * which is given to the callback in the second argument.
 **/
function walk(node, cb, to_visit = [node]) {
    const push = to_visit.push.bind(to_visit);
    while (to_visit.length) {
        const node = to_visit.pop();
        const ret = cb(node, to_visit);

        if (ret) {
            if (ret === walk_abort) return true;
            continue;
        }

        node._children_backwards(push);
    }
    return false;
}

const walk_abort = Symbol("abort walk");

/* -----[ TreeWalker ]----- */

class TreeWalker {
    constructor(callback) {
        this.visit = callback;
        this.stack = [];
        this.directives = Object.create(null);
    }

    _visit(node, descend) {
        this.push(node);
        var ret = this.visit(node, descend ? function() {
            descend.call(node);
        } : noop);
        if (!ret && descend) {
            descend.call(node);
        }
        this.pop();
        return ret;
    }

    parent(n) {
        return this.stack[this.stack.length - 2 - (n || 0)];
    }

    push(node) {
        if (node instanceof AST_Lambda) {
            this.directives = Object.create(this.directives);
        } else if (node instanceof AST_Directive && !this.directives[node.value]) {
            this.directives[node.value] = node;
        } else if (node instanceof AST_Class) {
            this.directives = Object.create(this.directives);
            if (!this.directives["use strict"]) {
                this.directives["use strict"] = node;
            }
        }
        this.stack.push(node);
    }

    pop() {
        var node = this.stack.pop();
        if (node instanceof AST_Lambda || node instanceof AST_Class) {
            this.directives = Object.getPrototypeOf(this.directives);
        }
    }

    self() {
        return this.stack[this.stack.length - 1];
    }

    find_parent(type) {
        var stack = this.stack;
        for (var i = stack.length; --i >= 0;) {
            var x = stack[i];
            if (x instanceof type) return x;
        }
    }

    find_scope() {
        var stack = this.stack;
        for (var i = stack.length; --i >= 0;) {
            const p = stack[i];
            if (p instanceof AST_Toplevel) return p;
            if (p instanceof AST_Lambda) return p;
            if (p.block_scope) return p.block_scope;
        }
    }

    has_directive(type) {
        var dir = this.directives[type];
        if (dir) return dir;
        var node = this.stack[this.stack.length - 1];
        if (node instanceof AST_Scope && node.body) {
            for (var i = 0; i < node.body.length; ++i) {
                var st = node.body[i];
                if (!(st instanceof AST_Directive)) break;
                if (st.value == type) return st;
            }
        }
    }

    loopcontrol_target(node) {
        var stack = this.stack;
        if (node.label) for (var i = stack.length; --i >= 0;) {
            var x = stack[i];
            if (x instanceof AST_LabeledStatement && x.label.name == node.label.name)
                return x.body;
        } else for (var i = stack.length; --i >= 0;) {
            var x = stack[i];
            if (x instanceof AST_IterationStatement
                || node instanceof AST_Break && x instanceof AST_Switch)
                return x;
        }
    }
}

// Tree transformer helpers.
class TreeTransformer extends TreeWalker {
    constructor(before, after) {
        super();
        this.before = before;
        this.after = after;
    }
}

const _PURE       = 0b00000001;
const _INLINE     = 0b00000010;
const _NOINLINE   = 0b00000100;
const _KEY        = 0b00001000;
const _MANGLEPROP = 0b00010000;

// XXX Emscripten: export TreeWalker for walking through AST in acorn-optimizer.mjs.
exports.TreeWalker = TreeWalker;

/***********************************************************************

  A JavaScript tokenizer / parser / beautifier / compressor.
  https://github.com/mishoo/UglifyJS2

  -------------------------------- (C) ---------------------------------

                           Author: Mihai Bazon
                         <mihai.bazon@gmail.com>
                       http://mihai.bazon.net/blog

  Distributed under the BSD license:

    Copyright 2012 (c) Mihai Bazon <mihai.bazon@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        * Redistributions of source code must retain the above
          copyright notice, this list of conditions and the following
          disclaimer.

        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials
          provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

 ***********************************************************************/

function def_transform(node, descend) {
    node.DEFMETHOD("transform", function(tw, in_list) {
        let transformed = undefined;
        tw.push(this);
        if (tw.before) transformed = tw.before(this, descend, in_list);
        if (transformed === undefined) {
            transformed = this;
            descend(transformed, tw);
            if (tw.after) {
                const after_ret = tw.after(transformed, in_list);
                if (after_ret !== undefined) transformed = after_ret;
            }
        }
        tw.pop();
        return transformed;
    });
}

def_transform(AST_Node, noop);

def_transform(AST_LabeledStatement, function(self, tw) {
    self.label = self.label.transform(tw);
    self.body = self.body.transform(tw);
});

def_transform(AST_SimpleStatement, function(self, tw) {
    self.body = self.body.transform(tw);
});

def_transform(AST_Block, function(self, tw) {
    self.body = MAP(self.body, tw);
});

def_transform(AST_Do, function(self, tw) {
    self.body = self.body.transform(tw);
    self.condition = self.condition.transform(tw);
});

def_transform(AST_While, function(self, tw) {
    self.condition = self.condition.transform(tw);
    self.body = self.body.transform(tw);
});

def_transform(AST_For, function(self, tw) {
    if (self.init) self.init = self.init.transform(tw);
    if (self.condition) self.condition = self.condition.transform(tw);
    if (self.step) self.step = self.step.transform(tw);
    self.body = self.body.transform(tw);
});

def_transform(AST_ForIn, function(self, tw) {
    self.init = self.init.transform(tw);
    self.object = self.object.transform(tw);
    self.body = self.body.transform(tw);
});

def_transform(AST_With, function(self, tw) {
    self.expression = self.expression.transform(tw);
    self.body = self.body.transform(tw);
});

def_transform(AST_Exit, function(self, tw) {
    if (self.value) self.value = self.value.transform(tw);
});

def_transform(AST_LoopControl, function(self, tw) {
    if (self.label) self.label = self.label.transform(tw);
});

def_transform(AST_If, function(self, tw) {
    self.condition = self.condition.transform(tw);
    self.body = self.body.transform(tw);
    if (self.alternative) self.alternative = self.alternative.transform(tw);
});

def_transform(AST_Switch, function(self, tw) {
    self.expression = self.expression.transform(tw);
    self.body = MAP(self.body, tw);
});

def_transform(AST_Case, function(self, tw) {
    self.expression = self.expression.transform(tw);
    self.body = MAP(self.body, tw);
});

def_transform(AST_Try, function(self, tw) {
    self.body = self.body.transform(tw);
    if (self.bcatch) self.bcatch = self.bcatch.transform(tw);
    if (self.bfinally) self.bfinally = self.bfinally.transform(tw);
});

def_transform(AST_Catch, function(self, tw) {
    if (self.argname) self.argname = self.argname.transform(tw);
    self.body = MAP(self.body, tw);
});

def_transform(AST_Definitions, function(self, tw) {
    self.definitions = MAP(self.definitions, tw);
});

def_transform(AST_VarDef, function(self, tw) {
    self.name = self.name.transform(tw);
    if (self.value) self.value = self.value.transform(tw);
});

def_transform(AST_Destructuring, function(self, tw) {
    self.names = MAP(self.names, tw);
});

def_transform(AST_Lambda, function(self, tw) {
    if (self.name) self.name = self.name.transform(tw);
    self.argnames = MAP(self.argnames, tw, /* allow_splicing */ false);
    if (self.body instanceof AST_Node) {
        self.body = self.body.transform(tw);
    } else {
        self.body = MAP(self.body, tw);
    }
});

def_transform(AST_Call, function(self, tw) {
    self.expression = self.expression.transform(tw);
    self.args = MAP(self.args, tw, /* allow_splicing */ false);
});

def_transform(AST_Sequence, function(self, tw) {
    const result = MAP(self.expressions, tw);
    self.expressions = result.length
        ? result
        : [new AST_Number({ value: 0 })];
});

def_transform(AST_PropAccess, function(self, tw) {
    self.expression = self.expression.transform(tw);
});

def_transform(AST_Sub, function(self, tw) {
    self.expression = self.expression.transform(tw);
    self.property = self.property.transform(tw);
});

def_transform(AST_Chain, function(self, tw) {
    self.expression = self.expression.transform(tw);
});

def_transform(AST_Yield, function(self, tw) {
    if (self.expression) self.expression = self.expression.transform(tw);
});

def_transform(AST_Await, function(self, tw) {
    self.expression = self.expression.transform(tw);
});

def_transform(AST_Unary, function(self, tw) {
    self.expression = self.expression.transform(tw);
});

def_transform(AST_Binary, function(self, tw) {
    self.left = self.left.transform(tw);
    self.right = self.right.transform(tw);
});

def_transform(AST_PrivateIn, function(self, tw) {
    self.key = self.key.transform(tw);
    self.value = self.value.transform(tw);
});

def_transform(AST_Conditional, function(self, tw) {
    self.condition = self.condition.transform(tw);
    self.consequent = self.consequent.transform(tw);
    self.alternative = self.alternative.transform(tw);
});

def_transform(AST_Array, function(self, tw) {
    self.elements = MAP(self.elements, tw);
});

def_transform(AST_Object, function(self, tw) {
    self.properties = MAP(self.properties, tw);
});

def_transform(AST_ObjectProperty, function(self, tw) {
    if (self.key instanceof AST_Node) {
        self.key = self.key.transform(tw);
    }
    if (self.value) self.value = self.value.transform(tw);
});

def_transform(AST_Class, function(self, tw) {
    if (self.name) self.name = self.name.transform(tw);
    if (self.extends) self.extends = self.extends.transform(tw);
    self.properties = MAP(self.properties, tw);
});

def_transform(AST_ClassStaticBlock, function(self, tw) {
    self.body = MAP(self.body, tw);
});

def_transform(AST_Expansion, function(self, tw) {
    self.expression = self.expression.transform(tw);
});

def_transform(AST_NameMapping, function(self, tw) {
    self.foreign_name = self.foreign_name.transform(tw);
    self.name = self.name.transform(tw);
});

def_transform(AST_Import, function(self, tw) {
    if (self.imported_name) self.imported_name = self.imported_name.transform(tw);
    if (self.imported_names) MAP(self.imported_names, tw);
    self.module_name = self.module_name.transform(tw);
});

def_transform(AST_Export, function(self, tw) {
    if (self.exported_definition) self.exported_definition = self.exported_definition.transform(tw);
    if (self.exported_value) self.exported_value = self.exported_value.transform(tw);
    if (self.exported_names) MAP(self.exported_names, tw);
    if (self.module_name) self.module_name = self.module_name.transform(tw);
});

def_transform(AST_TemplateString, function(self, tw) {
    self.segments = MAP(self.segments, tw);
});

def_transform(AST_PrefixedTemplateString, function(self, tw) {
    self.prefix = self.prefix.transform(tw);
    self.template_string = self.template_string.transform(tw);
});

/***********************************************************************

  A JavaScript tokenizer / parser / beautifier / compressor.
  https://github.com/mishoo/UglifyJS2

  -------------------------------- (C) ---------------------------------

                           Author: Mihai Bazon
                         <mihai.bazon@gmail.com>
                       http://mihai.bazon.net/blog

  Distributed under the BSD license:

    Copyright 2012 (c) Mihai Bazon <mihai.bazon@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        * Redistributions of source code must retain the above
          copyright notice, this list of conditions and the following
          disclaimer.

        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials
          provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

 ***********************************************************************/

(function() {

    var normalize_directives = function(body) {
        var in_directive = true;

        for (var i = 0; i < body.length; i++) {
            if (in_directive && body[i] instanceof AST_Statement && body[i].body instanceof AST_String) {
                body[i] = new AST_Directive({
                    start: body[i].start,
                    end: body[i].end,
                    value: body[i].body.value
                });
            } else if (in_directive && !(body[i] instanceof AST_Statement && body[i].body instanceof AST_String)) {
                in_directive = false;
            }
        }

        return body;
    };

    const assert_clause_from_moz = (assertions) => {
        if (assertions && assertions.length > 0) {
            return new AST_Object({
                start: my_start_token(assertions),
                end: my_end_token(assertions),
                properties: assertions.map((assertion_kv) =>
                    new AST_ObjectKeyVal({
                        start: my_start_token(assertion_kv),
                        end: my_end_token(assertion_kv),
                        key: assertion_kv.key.name || assertion_kv.key.value,
                        value: from_moz(assertion_kv.value)
                    })
                )
            });
        }
        return null;
    };

    var MOZ_TO_ME = {
        Program: function(M) {
            return new AST_Toplevel({
                start: my_start_token(M),
                end: my_end_token(M),
                body: normalize_directives(M.body.map(from_moz))
            });
        },

        ArrayPattern: function(M) {
            return new AST_Destructuring({
                start: my_start_token(M),
                end: my_end_token(M),
                names: M.elements.map(function(elm) {
                    if (elm === null) {
                        return new AST_Hole();
                    }
                    return from_moz(elm);
                }),
                is_array: true
            });
        },

        ObjectPattern: function(M) {
            return new AST_Destructuring({
                start: my_start_token(M),
                end: my_end_token(M),
                names: M.properties.map(from_moz),
                is_array: false
            });
        },

        AssignmentPattern: function(M) {
            return new AST_DefaultAssign({
                start: my_start_token(M),
                end: my_end_token(M),
                left: from_moz(M.left),
                operator: "=",
                right: from_moz(M.right)
            });
        },

        SpreadElement: function(M) {
            return new AST_Expansion({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.argument)
            });
        },

        RestElement: function(M) {
            return new AST_Expansion({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.argument)
            });
        },

        TemplateElement: function(M) {
            return new AST_TemplateSegment({
                start: my_start_token(M),
                end: my_end_token(M),
                value: M.value.cooked,
                raw: M.value.raw
            });
        },

        TemplateLiteral: function(M) {
            var segments = [];
            for (var i = 0; i < M.quasis.length; i++) {
                segments.push(from_moz(M.quasis[i]));
                if (M.expressions[i]) {
                    segments.push(from_moz(M.expressions[i]));
                }
            }
            return new AST_TemplateString({
                start: my_start_token(M),
                end: my_end_token(M),
                segments: segments
            });
        },

        TaggedTemplateExpression: function(M) {
            return new AST_PrefixedTemplateString({
                start: my_start_token(M),
                end: my_end_token(M),
                template_string: from_moz(M.quasi),
                prefix: from_moz(M.tag)
            });
        },

        FunctionDeclaration: function(M) {
            return new AST_Defun({
                start: my_start_token(M),
                end: my_end_token(M),
                name: from_moz(M.id),
                argnames: M.params.map(from_moz),
                is_generator: M.generator,
                async: M.async,
                body: normalize_directives(from_moz(M.body).body)
            });
        },

        FunctionExpression: function(M) {
            return new AST_Function({
                start: my_start_token(M),
                end: my_end_token(M),
                name: from_moz(M.id),
                argnames: M.params.map(from_moz),
                is_generator: M.generator,
                async: M.async,
                body: normalize_directives(from_moz(M.body).body)
            });
        },

        ArrowFunctionExpression: function(M) {
            const body = M.body.type === "BlockStatement"
                ? from_moz(M.body).body
                : [make_node(AST_Return, {}, { value: from_moz(M.body) })];
            return new AST_Arrow({
                start: my_start_token(M),
                end: my_end_token(M),
                argnames: M.params.map(from_moz),
                body,
                async: M.async,
            });
        },

        ExpressionStatement: function(M) {
            return new AST_SimpleStatement({
                start: my_start_token(M),
                end: my_end_token(M),
                body: from_moz(M.expression)
            });
        },

// XXX Emscripten localmod: Add a node type for a parenthesized expression so that we can retain
// Closure annotations that need a form "/**annotation*/(expression)"
        ParenthesizedExpression: function(M) {
            return new AST_ParenthesizedExpression({
                start: my_start_token(M),
                end: my_end_token(M),
                body: from_moz(M.expression)
            });
        },
// XXX End Emscripten localmod

        TryStatement: function(M) {
            var handlers = M.handlers || [M.handler];
            if (handlers.length > 1 || M.guardedHandlers && M.guardedHandlers.length) {
                throw new Error("Multiple catch clauses are not supported.");
            }
            return new AST_Try({
                start    : my_start_token(M),
                end      : my_end_token(M),
                body     : new AST_TryBlock(from_moz(M.block)),
                bcatch   : from_moz(handlers[0]),
                bfinally : M.finalizer ? new AST_Finally(from_moz(M.finalizer)) : null
            });
        },

        Property: function(M) {
            var key = M.key;
            var args = {
                start    : my_start_token(key || M.value),
                end      : my_end_token(M.value),
                key      : key.type == "Identifier" ? key.name : key.value,
                value    : from_moz(M.value)
            };
            if (M.computed) {
                args.key = from_moz(M.key);
            }
            if (M.method) {
                args.is_generator = M.value.generator;
                args.async = M.value.async;
                if (!M.computed) {
                    args.key = new AST_SymbolMethod({ name: args.key });
                } else {
                    args.key = from_moz(M.key);
                }
                return new AST_ConciseMethod(args);
            }
            if (M.kind == "init") {
                if (key.type != "Identifier" && key.type != "Literal") {
                    args.key = from_moz(key);
                }
                return new AST_ObjectKeyVal(args);
            }
            if (typeof args.key === "string" || typeof args.key === "number") {
                args.key = new AST_SymbolMethod({
                    name: args.key
                });
            }
            args.value = new AST_Accessor(args.value);
            if (M.kind == "get") return new AST_ObjectGetter(args);
            if (M.kind == "set") return new AST_ObjectSetter(args);
            if (M.kind == "method") {
                args.async = M.value.async;
                args.is_generator = M.value.generator;
                args.quote = M.computed ? "\"" : null;
                return new AST_ConciseMethod(args);
            }
        },

        MethodDefinition: function(M) {
            const is_private = M.key.type === "PrivateIdentifier";
            const key = M.computed ? from_moz(M.key) : new AST_SymbolMethod({ name: M.key.name || M.key.value });

            var args = {
                start    : my_start_token(M),
                end      : my_end_token(M),
                key,
                value    : from_moz(M.value),
                static   : M.static,
            };
            if (M.kind == "get") {
                return new (is_private ? AST_PrivateGetter : AST_ObjectGetter)(args);
            }
            if (M.kind == "set") {
                return new (is_private ? AST_PrivateSetter : AST_ObjectSetter)(args);
            }
            args.is_generator = M.value.generator;
            args.async = M.value.async;
            return new (is_private ? AST_PrivateMethod : AST_ConciseMethod)(args);
        },

        FieldDefinition: function(M) {
            let key;
            if (M.computed) {
                key = from_moz(M.key);
            } else {
                if (M.key.type !== "Identifier") throw new Error("Non-Identifier key in FieldDefinition");
                key = from_moz(M.key);
            }
            return new AST_ClassProperty({
                start    : my_start_token(M),
                end      : my_end_token(M),
                key,
                value    : from_moz(M.value),
                static   : M.static,
            });
        },

        PropertyDefinition: function(M) {
            let key;
            if (M.computed) {
                key = from_moz(M.key);
            } else if (M.key.type === "PrivateIdentifier") {
                return new AST_ClassPrivateProperty({
                    start    : my_start_token(M),
                    end      : my_end_token(M),
                    key      : from_moz(M.key),
                    value    : from_moz(M.value),
                    static   : M.static,
                });
            } else {
                if (M.key.type !== "Identifier") {
                    throw new Error("Non-Identifier key in PropertyDefinition");
                }
                key = from_moz(M.key);
            }

            return new AST_ClassProperty({
                start    : my_start_token(M),
                end      : my_end_token(M),
                key,
                value    : from_moz(M.value),
                static   : M.static,
            });
        },

        PrivateIdentifier: function (M) {
            return new AST_SymbolPrivateProperty({
                start: my_start_token(M),
                end: my_end_token(M),
                name: M.name
            });
        },

        StaticBlock: function(M) {
            return new AST_ClassStaticBlock({
                start : my_start_token(M),
                end   : my_end_token(M),
                body  : M.body.map(from_moz),
            });
        },

        ArrayExpression: function(M) {
            return new AST_Array({
                start    : my_start_token(M),
                end      : my_end_token(M),
                elements : M.elements.map(function(elem) {
                    return elem === null ? new AST_Hole() : from_moz(elem);
                })
            });
        },

        ObjectExpression: function(M) {
            return new AST_Object({
                start      : my_start_token(M),
                end        : my_end_token(M),
                properties : M.properties.map(function(prop) {
                    if (prop.type === "SpreadElement") {
                        return from_moz(prop);
                    }
                    prop.type = "Property";
                    // XXX EMSCRIPTEN preserve quoted properties
                    // https://github.com/mishoo/UglifyJS2/pull/3323
                    var ret = from_moz(prop);
                    if (prop.key.type === "Literal" &&
                        (prop.key.raw[0] === '"' || prop.key.raw[0] === "'")) {
                        ret.quote = true;
                    }
                    return ret;
                })
            });
        },

        SequenceExpression: function(M) {
            return new AST_Sequence({
                start      : my_start_token(M),
                end        : my_end_token(M),
                expressions: M.expressions.map(from_moz)
            });
        },

        MemberExpression: function(M) {
            if (M.property.type === "PrivateIdentifier") {
                return new AST_DotHash({
                    start      : my_start_token(M),
                    end        : my_end_token(M),
                    property   : M.property.name,
                    expression : from_moz(M.object),
                    optional   : M.optional || false
                });
            }
            return new (M.computed ? AST_Sub : AST_Dot)({
                start      : my_start_token(M),
                end        : my_end_token(M),
                property   : M.computed ? from_moz(M.property) : M.property.name,
                expression : from_moz(M.object),
                optional   : M.optional || false
            });
        },

        ChainExpression: function(M) {
            return new AST_Chain({
                start      : my_start_token(M),
                end        : my_end_token(M),
                expression : from_moz(M.expression)
            });
        },

        SwitchCase: function(M) {
            return new (M.test ? AST_Case : AST_Default)({
                start      : my_start_token(M),
                end        : my_end_token(M),
                expression : from_moz(M.test),
                body       : M.consequent.map(from_moz)
            });
        },

        VariableDeclaration: function(M) {
            return new (M.kind === "const" ? AST_Const :
                        M.kind === "let" ? AST_Let : AST_Var)({
                start       : my_start_token(M),
                end         : my_end_token(M),
                definitions : M.declarations.map(from_moz)
            });
        },

        ImportDeclaration: function(M) {
            var imported_name = null;
            var imported_names = null;
            M.specifiers.forEach(function (specifier) {
                if (specifier.type === "ImportSpecifier" || specifier.type === "ImportNamespaceSpecifier") {
                    if (!imported_names) { imported_names = []; }
                    imported_names.push(from_moz(specifier));
                } else if (specifier.type === "ImportDefaultSpecifier") {
                    imported_name = from_moz(specifier);
                }
            });
            return new AST_Import({
                start       : my_start_token(M),
                end         : my_end_token(M),
                imported_name: imported_name,
                imported_names : imported_names,
                module_name : from_moz(M.source),
                assert_clause: assert_clause_from_moz(M.assertions)
            });
        },

        ImportSpecifier: function(M) {
            return new AST_NameMapping({
                start: my_start_token(M),
                end: my_end_token(M),
                foreign_name: from_moz(M.imported),
                name: from_moz(M.local)
            });
        },

        ImportDefaultSpecifier: function(M) {
            return from_moz(M.local);
        },

        ImportNamespaceSpecifier: function(M) {
            return new AST_NameMapping({
                start: my_start_token(M),
                end: my_end_token(M),
                foreign_name: new AST_SymbolImportForeign({ name: "*" }),
                name: from_moz(M.local)
            });
        },

        ExportAllDeclaration: function(M) {
            var foreign_name = M.exported == null ? 
                new AST_SymbolExportForeign({ name: "*" }) :
                from_moz(M.exported);
            return new AST_Export({
                start: my_start_token(M),
                end: my_end_token(M),
                exported_names: [
                    new AST_NameMapping({
                        name: new AST_SymbolExportForeign({ name: "*" }),
                        foreign_name: foreign_name
                    })
                ],
                module_name: from_moz(M.source),
                assert_clause: assert_clause_from_moz(M.assertions)
            });
        },

        ExportNamedDeclaration: function(M) {
            return new AST_Export({
                start: my_start_token(M),
                end: my_end_token(M),
                exported_definition: from_moz(M.declaration),
                exported_names: M.specifiers && M.specifiers.length ? M.specifiers.map(function (specifier) {
                    return from_moz(specifier);
                }) : null,
                module_name: from_moz(M.source),
                assert_clause: assert_clause_from_moz(M.assertions)
            });
        },

        ExportDefaultDeclaration: function(M) {
            return new AST_Export({
                start: my_start_token(M),
                end: my_end_token(M),
                exported_value: from_moz(M.declaration),
                is_default: true
            });
        },

        ExportSpecifier: function(M) {
            return new AST_NameMapping({
                foreign_name: from_moz(M.exported),
                name: from_moz(M.local)
            });
        },

        Literal: function(M) {
            var val = M.value, args = {
                start  : my_start_token(M),
                end    : my_end_token(M)
            };
            var rx = M.regex;
            if (rx && rx.pattern) {
                // RegExpLiteral as per ESTree AST spec
                args.value = {
                    source: rx.pattern,
                    flags: rx.flags
                };
                return new AST_RegExp(args);
            } else if (rx) {
                // support legacy RegExp
                const rx_source = M.raw || val;
                const match = rx_source.match(/^\/(.*)\/(\w*)$/);
                if (!match) throw new Error("Invalid regex source " + rx_source);
                const [_, source, flags] = match;
                args.value = { source, flags };
                return new AST_RegExp(args);
            }
            if (val === null) return new AST_Null(args);
            switch (typeof val) {
              case "string":
                args.quote = "\"";
                var p = FROM_MOZ_STACK[FROM_MOZ_STACK.length - 2];
                if (p.type == "ImportSpecifier") {
                    args.name = val;
                    return new AST_SymbolImportForeign(args);
                } else if (p.type == "ExportSpecifier") {
                    args.name = val;
                    if (M == p.exported) {
                        return new AST_SymbolExportForeign(args);
                    } else {
                        return new AST_SymbolExport(args);
                    }
                } else if (p.type == "ExportAllDeclaration" && M == p.exported) {
                    args.name = val;
                    return new AST_SymbolExportForeign(args);
                }
                args.value = val;
                return new AST_String(args);
              case "number":
                args.value = val;
                args.raw = M.raw || val.toString();
                return new AST_Number(args);
              case "boolean":
                return new (val ? AST_True : AST_False)(args);
              case "bigint":
                args.value = val;
                return new AST_BigInt(args);
              case "undefined":
                return undefined;
              default:
                throw new Error("Unhandled value type: " + typeof val);
            }
        },

        MetaProperty: function(M) {
            if (M.meta.name === "new" && M.property.name === "target") {
                return new AST_NewTarget({
                    start: my_start_token(M),
                    end: my_end_token(M)
                });
            } else if (M.meta.name === "import" && M.property.name === "meta") {
                return new AST_ImportMeta({
                    start: my_start_token(M),
                    end: my_end_token(M)
                });
            }
        },

        Identifier: function(M) {
            var p = FROM_MOZ_STACK[FROM_MOZ_STACK.length - 2];
            return new (  p.type == "LabeledStatement" ? AST_Label
                        : p.type == "VariableDeclarator" && p.id === M ? (p.kind == "const" ? AST_SymbolConst : p.kind == "let" ? AST_SymbolLet : AST_SymbolVar)
                        : /Import.*Specifier/.test(p.type) ? (p.local === M ? AST_SymbolImport : AST_SymbolImportForeign)
                        : p.type == "ExportSpecifier" ? (p.local === M ? AST_SymbolExport : AST_SymbolExportForeign)
                        : p.type == "FunctionExpression" ? (p.id === M ? AST_SymbolLambda : AST_SymbolFunarg)
                        : p.type == "FunctionDeclaration" ? (p.id === M ? AST_SymbolDefun : AST_SymbolFunarg)
                        : p.type == "ArrowFunctionExpression" ? (p.params.includes(M)) ? AST_SymbolFunarg : AST_SymbolRef
                        : p.type == "ClassExpression" ? (p.id === M ? AST_SymbolClass : AST_SymbolRef)
                        : p.type == "Property" ? (p.key === M && p.computed || p.value === M ? AST_SymbolRef : AST_SymbolMethod)
                        : p.type == "PropertyDefinition" || p.type === "FieldDefinition" ? (p.key === M && p.computed || p.value === M ? AST_SymbolRef : AST_SymbolClassProperty)
                        : p.type == "ClassDeclaration" ? (p.id === M ? AST_SymbolDefClass : AST_SymbolRef)
                        : p.type == "MethodDefinition" ? (p.computed ? AST_SymbolRef : AST_SymbolMethod)
                        : p.type == "CatchClause" ? AST_SymbolCatch
                        : p.type == "BreakStatement" || p.type == "ContinueStatement" ? AST_LabelRef
                        : AST_SymbolRef)({
                            start : my_start_token(M),
                            end   : my_end_token(M),
                            name  : M.name
                        });
        },

        BigIntLiteral(M) {
            return new AST_BigInt({
                start : my_start_token(M),
                end   : my_end_token(M),
                value : M.value
            });
        },

        EmptyStatement: function(M) {
            return new AST_EmptyStatement({
                start: my_start_token(M),
                end: my_end_token(M)
            });
        },

        BlockStatement: function(M) {
            return new AST_BlockStatement({
                start: my_start_token(M),
                end: my_end_token(M),
                body: M.body.map(from_moz)
            });
        },

        IfStatement: function(M) {
            return new AST_If({
                start: my_start_token(M),
                end: my_end_token(M),
                condition: from_moz(M.test),
                body: from_moz(M.consequent),
                alternative: from_moz(M.alternate)
            });
        },

        LabeledStatement: function(M) {
            return new AST_LabeledStatement({
                start: my_start_token(M),
                end: my_end_token(M),
                label: from_moz(M.label),
                body: from_moz(M.body)
            });
        },

        BreakStatement: function(M) {
            return new AST_Break({
                start: my_start_token(M),
                end: my_end_token(M),
                label: from_moz(M.label)
            });
        },

        ContinueStatement: function(M) {
            return new AST_Continue({
                start: my_start_token(M),
                end: my_end_token(M),
                label: from_moz(M.label)
            });
        },

        WithStatement: function(M) {
            return new AST_With({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.object),
                body: from_moz(M.body)
            });
        },

        SwitchStatement: function(M) {
            return new AST_Switch({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.discriminant),
                body: M.cases.map(from_moz)
            });
        },

        ReturnStatement: function(M) {
            return new AST_Return({
                start: my_start_token(M),
                end: my_end_token(M),
                value: from_moz(M.argument)
            });
        },

        ThrowStatement: function(M) {
            return new AST_Throw({
                start: my_start_token(M),
                end: my_end_token(M),
                value: from_moz(M.argument)
            });
        },

        WhileStatement: function(M) {
            return new AST_While({
                start: my_start_token(M),
                end: my_end_token(M),
                condition: from_moz(M.test),
                body: from_moz(M.body)
            });
        },

        DoWhileStatement: function(M) {
            return new AST_Do({
                start: my_start_token(M),
                end: my_end_token(M),
                condition: from_moz(M.test),
                body: from_moz(M.body)
            });
        },

        ForStatement: function(M) {
            return new AST_For({
                start: my_start_token(M),
                end: my_end_token(M),
                init: from_moz(M.init),
                condition: from_moz(M.test),
                step: from_moz(M.update),
                body: from_moz(M.body)
            });
        },

        ForInStatement: function(M) {
            return new AST_ForIn({
                start: my_start_token(M),
                end: my_end_token(M),
                init: from_moz(M.left),
                object: from_moz(M.right),
                body: from_moz(M.body)
            });
        },

        ForOfStatement: function(M) {
            return new AST_ForOf({
                start: my_start_token(M),
                end: my_end_token(M),
                init: from_moz(M.left),
                object: from_moz(M.right),
                body: from_moz(M.body),
                await: M.await
            });
        },

        AwaitExpression: function(M) {
            return new AST_Await({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.argument)
            });
        },

        YieldExpression: function(M) {
            return new AST_Yield({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.argument),
                is_star: M.delegate
            });
        },

        DebuggerStatement: function(M) {
            return new AST_Debugger({
                start: my_start_token(M),
                end: my_end_token(M)
            });
        },

        VariableDeclarator: function(M) {
            return new AST_VarDef({
                start: my_start_token(M),
                end: my_end_token(M),
                name: from_moz(M.id),
                value: from_moz(M.init)
            });
        },

        CatchClause: function(M) {
            return new AST_Catch({
                start: my_start_token(M),
                end: my_end_token(M),
                argname: from_moz(M.param),
                body: from_moz(M.body).body
            });
        },

        ThisExpression: function(M) {
            return new AST_This({
                start: my_start_token(M),
                end: my_end_token(M)
            });
        },

        Super: function(M) {
            return new AST_Super({
                start: my_start_token(M),
                end: my_end_token(M)
            });
        },

        BinaryExpression: function(M) {
            if (M.left.type === "PrivateIdentifier") {
                return new AST_PrivateIn({
                    start: my_start_token(M),
                    end: my_end_token(M),
                    key: new AST_SymbolPrivateProperty({
                        start: my_start_token(M.left),
                        end: my_end_token(M.left),
                        name: M.left.name
                    }),
                    value: from_moz(M.right),
                });
            }
            return new AST_Binary({
                start: my_start_token(M),
                end: my_end_token(M),
                operator: M.operator,
                left: from_moz(M.left),
                right: from_moz(M.right)
            });
        },

        LogicalExpression: function(M) {
            return new AST_Binary({
                start: my_start_token(M),
                end: my_end_token(M),
                operator: M.operator,
                left: from_moz(M.left),
                right: from_moz(M.right)
            });
        },

        AssignmentExpression: function(M) {
            return new AST_Assign({
                start: my_start_token(M),
                end: my_end_token(M),
                operator: M.operator,
                left: from_moz(M.left),
                right: from_moz(M.right)
            });
        },

        ConditionalExpression: function(M) {
            return new AST_Conditional({
                start: my_start_token(M),
                end: my_end_token(M),
                condition: from_moz(M.test),
                consequent: from_moz(M.consequent),
                alternative: from_moz(M.alternate)
            });
        },

        NewExpression: function(M) {
            return new AST_New({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.callee),
                args: M.arguments.map(from_moz)
            });
        },

        CallExpression: function(M) {
            return new AST_Call({
                start: my_start_token(M),
                end: my_end_token(M),
                expression: from_moz(M.callee),
                optional: M.optional,
                args: M.arguments.map(from_moz)
            });
        },

        ImportExpression: function(M) {
          let import_token = my_start_token(M);
          return new AST_Call({
            start      : import_token,
            end        : my_end_token(M),
            expression : new AST_SymbolRef({
              start    : import_token,
              end      : import_token,
              name     : "import"
            }),
            args       : [from_moz(M.source)]
          });
        }
    };

    MOZ_TO_ME.UpdateExpression =
    MOZ_TO_ME.UnaryExpression = function To_Moz_Unary(M) {
        var prefix = "prefix" in M ? M.prefix
            : M.type == "UnaryExpression" ? true : false;
        return new (prefix ? AST_UnaryPrefix : AST_UnaryPostfix)({
            start      : my_start_token(M),
            end        : my_end_token(M),
            operator   : M.operator,
            expression : from_moz(M.argument)
        });
    };

    MOZ_TO_ME.ClassDeclaration =
    MOZ_TO_ME.ClassExpression = function From_Moz_Class(M) {
        return new (M.type === "ClassDeclaration" ? AST_DefClass : AST_ClassExpression)({
            start    : my_start_token(M),
            end      : my_end_token(M),
            name     : from_moz(M.id),
            extends  : from_moz(M.superClass),
            properties: M.body.body.map(from_moz)
        });
    };

    def_to_moz(AST_EmptyStatement, function To_Moz_EmptyStatement() {
        return {
            type: "EmptyStatement"
        };
    });
    def_to_moz(AST_BlockStatement, function To_Moz_BlockStatement(M) {
        return {
            type: "BlockStatement",
            body: M.body.map(to_moz)
        };
    });
    def_to_moz(AST_If, function To_Moz_IfStatement(M) {
        return {
            type: "IfStatement",
            test: to_moz(M.condition),
            consequent: to_moz(M.body),
            alternate: to_moz(M.alternative)
        };
    });
    def_to_moz(AST_LabeledStatement, function To_Moz_LabeledStatement(M) {
        return {
            type: "LabeledStatement",
            label: to_moz(M.label),
            body: to_moz(M.body)
        };
    });
    def_to_moz(AST_Break, function To_Moz_BreakStatement(M) {
        return {
            type: "BreakStatement",
            label: to_moz(M.label)
        };
    });
    def_to_moz(AST_Continue, function To_Moz_ContinueStatement(M) {
        return {
            type: "ContinueStatement",
            label: to_moz(M.label)
        };
    });
    def_to_moz(AST_With, function To_Moz_WithStatement(M) {
        return {
            type: "WithStatement",
            object: to_moz(M.expression),
            body: to_moz(M.body)
        };
    });
    def_to_moz(AST_Switch, function To_Moz_SwitchStatement(M) {
        return {
            type: "SwitchStatement",
            discriminant: to_moz(M.expression),
            cases: M.body.map(to_moz)
        };
    });
    def_to_moz(AST_Return, function To_Moz_ReturnStatement(M) {
        return {
            type: "ReturnStatement",
            argument: to_moz(M.value)
        };
    });
    def_to_moz(AST_Throw, function To_Moz_ThrowStatement(M) {
        return {
            type: "ThrowStatement",
            argument: to_moz(M.value)
        };
    });
    def_to_moz(AST_While, function To_Moz_WhileStatement(M) {
        return {
            type: "WhileStatement",
            test: to_moz(M.condition),
            body: to_moz(M.body)
        };
    });
    def_to_moz(AST_Do, function To_Moz_DoWhileStatement(M) {
        return {
            type: "DoWhileStatement",
            test: to_moz(M.condition),
            body: to_moz(M.body)
        };
    });
    def_to_moz(AST_For, function To_Moz_ForStatement(M) {
        return {
            type: "ForStatement",
            init: to_moz(M.init),
            test: to_moz(M.condition),
            update: to_moz(M.step),
            body: to_moz(M.body)
        };
    });
    def_to_moz(AST_ForIn, function To_Moz_ForInStatement(M) {
        return {
            type: "ForInStatement",
            left: to_moz(M.init),
            right: to_moz(M.object),
            body: to_moz(M.body)
        };
    });
    def_to_moz(AST_ForOf, function To_Moz_ForOfStatement(M) {
        return {
            type: "ForOfStatement",
            left: to_moz(M.init),
            right: to_moz(M.object),
            body: to_moz(M.body),
            await: M.await
        };
    });
    def_to_moz(AST_Await, function To_Moz_AwaitExpression(M) {
        return {
            type: "AwaitExpression",
            argument: to_moz(M.expression)
        };
    });
    def_to_moz(AST_Yield, function To_Moz_YieldExpression(M) {
        return {
            type: "YieldExpression",
            argument: to_moz(M.expression),
            delegate: M.is_star
        };
    });
    def_to_moz(AST_Debugger, function To_Moz_DebuggerStatement() {
        return {
            type: "DebuggerStatement"
        };
    });
    def_to_moz(AST_VarDef, function To_Moz_VariableDeclarator(M) {
        return {
            type: "VariableDeclarator",
            id: to_moz(M.name),
            init: to_moz(M.value)
        };
    });
    def_to_moz(AST_Catch, function To_Moz_CatchClause(M) {
        return {
            type: "CatchClause",
            param: to_moz(M.argname),
            body: to_moz_block(M)
        };
    });

    def_to_moz(AST_This, function To_Moz_ThisExpression() {
        return {
            type: "ThisExpression"
        };
    });
    def_to_moz(AST_Super, function To_Moz_Super() {
        return {
            type: "Super"
        };
    });
    def_to_moz(AST_Binary, function To_Moz_BinaryExpression(M) {
        return {
            type: "BinaryExpression",
            operator: M.operator,
            left: to_moz(M.left),
            right: to_moz(M.right)
        };
    });
    def_to_moz(AST_Binary, function To_Moz_LogicalExpression(M) {
        return {
            type: "LogicalExpression",
            operator: M.operator,
            left: to_moz(M.left),
            right: to_moz(M.right)
        };
    });
    def_to_moz(AST_Assign, function To_Moz_AssignmentExpression(M) {
        return {
            type: "AssignmentExpression",
            operator: M.operator,
            left: to_moz(M.left),
            right: to_moz(M.right)
        };
    });
    def_to_moz(AST_Conditional, function To_Moz_ConditionalExpression(M) {
        return {
            type: "ConditionalExpression",
            test: to_moz(M.condition),
            consequent: to_moz(M.consequent),
            alternate: to_moz(M.alternative)
        };
    });
    def_to_moz(AST_New, function To_Moz_NewExpression(M) {
        return {
            type: "NewExpression",
            callee: to_moz(M.expression),
            arguments: M.args.map(to_moz)
        };
    });
    def_to_moz(AST_Call, function To_Moz_CallExpression(M) {
        return {
            type: "CallExpression",
            callee: to_moz(M.expression),
            optional: M.optional,
            arguments: M.args.map(to_moz)
        };
    });

    def_to_moz(AST_Toplevel, function To_Moz_Program(M) {
        return to_moz_scope("Program", M);
    });

    def_to_moz(AST_Expansion, function To_Moz_Spread(M) {
        return {
            type: to_moz_in_destructuring() ? "RestElement" : "SpreadElement",
            argument: to_moz(M.expression)
        };
    });

    def_to_moz(AST_PrefixedTemplateString, function To_Moz_TaggedTemplateExpression(M) {
        return {
            type: "TaggedTemplateExpression",
            tag: to_moz(M.prefix),
            quasi: to_moz(M.template_string)
        };
    });

    def_to_moz(AST_TemplateString, function To_Moz_TemplateLiteral(M) {
        var quasis = [];
        var expressions = [];
        for (var i = 0; i < M.segments.length; i++) {
            if (i % 2 !== 0) {
                expressions.push(to_moz(M.segments[i]));
            } else {
                quasis.push({
                    type: "TemplateElement",
                    value: {
                        raw: M.segments[i].raw,
                        cooked: M.segments[i].value
                    },
                    tail: i === M.segments.length - 1
                });
            }
        }
        return {
            type: "TemplateLiteral",
            quasis: quasis,
            expressions: expressions
        };
    });

    def_to_moz(AST_Defun, function To_Moz_FunctionDeclaration(M) {
        return {
            type: "FunctionDeclaration",
            id: to_moz(M.name),
            params: M.argnames.map(to_moz),
            generator: M.is_generator,
            async: M.async,
            body: to_moz_scope("BlockStatement", M)
        };
    });

    def_to_moz(AST_Function, function To_Moz_FunctionExpression(M, parent) {
        var is_generator = parent.is_generator !== undefined ?
            parent.is_generator : M.is_generator;
        return {
            type: "FunctionExpression",
            id: to_moz(M.name),
            params: M.argnames.map(to_moz),
            generator: is_generator,
            async: M.async,
            body: to_moz_scope("BlockStatement", M)
        };
    });

    def_to_moz(AST_Arrow, function To_Moz_ArrowFunctionExpression(M) {
        var body = {
            type: "BlockStatement",
            body: M.body.map(to_moz)
        };
        return {
            type: "ArrowFunctionExpression",
            params: M.argnames.map(to_moz),
            async: M.async,
            body: body
        };
    });

    def_to_moz(AST_Destructuring, function To_Moz_ObjectPattern(M) {
        if (M.is_array) {
            return {
                type: "ArrayPattern",
                elements: M.names.map(to_moz)
            };
        }
        return {
            type: "ObjectPattern",
            properties: M.names.map(to_moz)
        };
    });

    def_to_moz(AST_Directive, function To_Moz_Directive(M) {
        return {
            type: "ExpressionStatement",
            expression: {
                type: "Literal",
                value: M.value,
                raw: M.print_to_string()
            },
            directive: M.value
        };
    });

    def_to_moz(AST_SimpleStatement, function To_Moz_ExpressionStatement(M) {
        return {
            type: "ExpressionStatement",
            expression: to_moz(M.body)
        };
    });

    def_to_moz(AST_SwitchBranch, function To_Moz_SwitchCase(M) {
        return {
            type: "SwitchCase",
            test: to_moz(M.expression),
            consequent: M.body.map(to_moz)
        };
    });

    def_to_moz(AST_Try, function To_Moz_TryStatement(M) {
        return {
            type: "TryStatement",
            block: to_moz_block(M.body),
            handler: to_moz(M.bcatch),
            guardedHandlers: [],
            finalizer: to_moz(M.bfinally)
        };
    });

    def_to_moz(AST_Catch, function To_Moz_CatchClause(M) {
        return {
            type: "CatchClause",
            param: to_moz(M.argname),
            guard: null,
            body: to_moz_block(M)
        };
    });

    def_to_moz(AST_Definitions, function To_Moz_VariableDeclaration(M) {
        return {
            type: "VariableDeclaration",
            kind:
                M instanceof AST_Const ? "const" :
                M instanceof AST_Let ? "let" : "var",
            declarations: M.definitions.map(to_moz)
        };
    });

    const assert_clause_to_moz = assert_clause => {
        const assertions = [];
        if (assert_clause) {
            for (const { key, value } of assert_clause.properties) {
                const key_moz = is_basic_identifier_string(key)
                    ? { type: "Identifier", name: key }
                    : { type: "Literal", value: key, raw: JSON.stringify(key) };
                assertions.push({
                    type: "ImportAttribute",
                    key: key_moz,
                    value: to_moz(value)
                });
            }
        }
        return assertions;
    };

    def_to_moz(AST_Export, function To_Moz_ExportDeclaration(M) {
        if (M.exported_names) {
            var first_exported = M.exported_names[0];
            var first_exported_name = first_exported.name;
            if (first_exported_name.name === "*" && !first_exported_name.quote) {
                var foreign_name = first_exported.foreign_name;
                var exported = foreign_name.name === "*" && !foreign_name.quote
                    ? null
                    : to_moz(foreign_name);
                return {
                    type: "ExportAllDeclaration",
                    source: to_moz(M.module_name),
                    exported: exported,
                    assertions: assert_clause_to_moz(M.assert_clause)
                };
            }
            return {
                type: "ExportNamedDeclaration",
                specifiers: M.exported_names.map(function (name_mapping) {
                    return {
                        type: "ExportSpecifier",
                        exported: to_moz(name_mapping.foreign_name),
                        local: to_moz(name_mapping.name)
                    };
                }),
                declaration: to_moz(M.exported_definition),
                source: to_moz(M.module_name),
                assertions: assert_clause_to_moz(M.assert_clause)
            };
        }
        return {
            type: M.is_default ? "ExportDefaultDeclaration" : "ExportNamedDeclaration",
            declaration: to_moz(M.exported_value || M.exported_definition)
        };
    });

    def_to_moz(AST_Import, function To_Moz_ImportDeclaration(M) {
        var specifiers = [];
        if (M.imported_name) {
            specifiers.push({
                type: "ImportDefaultSpecifier",
                local: to_moz(M.imported_name)
            });
        }
        if (M.imported_names) {
            var first_imported_foreign_name = M.imported_names[0].foreign_name;
            if (first_imported_foreign_name.name === "*" && !first_imported_foreign_name.quote) {
                specifiers.push({
                    type: "ImportNamespaceSpecifier",
                    local: to_moz(M.imported_names[0].name)
                });
            } else {
                M.imported_names.forEach(function(name_mapping) {
                    specifiers.push({
                        type: "ImportSpecifier",
                        local: to_moz(name_mapping.name),
                        imported: to_moz(name_mapping.foreign_name)
                    });
                });
            }
        }
        return {
            type: "ImportDeclaration",
            specifiers: specifiers,
            source: to_moz(M.module_name),
            assertions: assert_clause_to_moz(M.assert_clause)
        };
    });

    def_to_moz(AST_ImportMeta, function To_Moz_MetaProperty() {
        return {
            type: "MetaProperty",
            meta: {
                type: "Identifier",
                name: "import"
            },
            property: {
                type: "Identifier",
                name: "meta"
            }
        };
    });

    def_to_moz(AST_Sequence, function To_Moz_SequenceExpression(M) {
        return {
            type: "SequenceExpression",
            expressions: M.expressions.map(to_moz)
        };
    });

    def_to_moz(AST_DotHash, function To_Moz_PrivateMemberExpression(M) {
        return {
            type: "MemberExpression",
            object: to_moz(M.expression),
            computed: false,
            property: {
                type: "PrivateIdentifier",
                name: M.property
            },
            optional: M.optional
        };
    });

    def_to_moz(AST_PropAccess, function To_Moz_MemberExpression(M) {
        var isComputed = M instanceof AST_Sub;
        return {
            type: "MemberExpression",
            object: to_moz(M.expression),
            computed: isComputed,
            property: isComputed ? to_moz(M.property) : {type: "Identifier", name: M.property},
            optional: M.optional
        };
    });

    def_to_moz(AST_Chain, function To_Moz_ChainExpression(M) {
        return {
            type: "ChainExpression",
            expression: to_moz(M.expression)
        };
    });

    def_to_moz(AST_Unary, function To_Moz_Unary(M) {
        return {
            type: M.operator == "++" || M.operator == "--" ? "UpdateExpression" : "UnaryExpression",
            operator: M.operator,
            prefix: M instanceof AST_UnaryPrefix,
            argument: to_moz(M.expression)
        };
    });

    def_to_moz(AST_Binary, function To_Moz_BinaryExpression(M) {
        if (M.operator == "=" && to_moz_in_destructuring()) {
            return {
                type: "AssignmentPattern",
                left: to_moz(M.left),
                right: to_moz(M.right)
            };
        }

        const type = M.operator == "&&" || M.operator == "||" || M.operator === "??"
            ? "LogicalExpression"
            : "BinaryExpression";

        return {
            type,
            left: to_moz(M.left),
            operator: M.operator,
            right: to_moz(M.right)
        };
    });

    def_to_moz(AST_PrivateIn, function To_Moz_BinaryExpression_PrivateIn(M) {
        return {
            type: "BinaryExpression",
            left: { type: "PrivateIdentifier", name: M.key.name },
            operator: "in",
            right: to_moz(M.value),
        };
    });

    def_to_moz(AST_Array, function To_Moz_ArrayExpression(M) {
        return {
            type: "ArrayExpression",
            elements: M.elements.map(to_moz)
        };
    });

    def_to_moz(AST_Object, function To_Moz_ObjectExpression(M) {
        return {
            type: "ObjectExpression",
            properties: M.properties.map(to_moz)
        };
    });

    def_to_moz(AST_ObjectProperty, function To_Moz_Property(M, parent) {
        var key = M.key instanceof AST_Node ? to_moz(M.key) : {
            type: "Identifier",
            value: M.key
        };
        if (typeof M.key === "number") {
            key = {
                type: "Literal",
                value: Number(M.key)
            };
        }
        if (typeof M.key === "string") {
            key = {
                type: "Identifier",
                name: M.key
            };
        }
        var kind;
        var string_or_num = typeof M.key === "string" || typeof M.key === "number";
        var computed = string_or_num ? false : !(M.key instanceof AST_Symbol) || M.key instanceof AST_SymbolRef;
        if (M instanceof AST_ObjectKeyVal) {
            kind = "init";
            computed = !string_or_num;
        } else
        if (M instanceof AST_ObjectGetter) {
            kind = "get";
        } else
        if (M instanceof AST_ObjectSetter) {
            kind = "set";
        }
        if (M instanceof AST_PrivateGetter || M instanceof AST_PrivateSetter) {
            const kind = M instanceof AST_PrivateGetter ? "get" : "set";
            return {
                type: "MethodDefinition",
                computed: false,
                kind: kind,
                static: M.static,
                key: {
                    type: "PrivateIdentifier",
                    name: M.key.name
                },
                value: to_moz(M.value)
            };
        }
        if (M instanceof AST_ClassPrivateProperty) {
            return {
                type: "PropertyDefinition",
                key: {
                    type: "PrivateIdentifier",
                    name: M.key.name
                },
                value: to_moz(M.value),
                computed: false,
                static: M.static
            };
        }
        if (M instanceof AST_ClassProperty) {
            return {
                type: "PropertyDefinition",
                key,
                value: to_moz(M.value),
                computed,
                static: M.static
            };
        }
        if (parent instanceof AST_Class) {
            return {
                type: "MethodDefinition",
                computed: computed,
                kind: kind,
                static: M.static,
                key: to_moz(M.key),
                value: to_moz(M.value)
            };
        }
        return {
            type: "Property",
            computed: computed,
            kind: kind,
            key: key,
            value: to_moz(M.value)
        };
    });

    def_to_moz(AST_ConciseMethod, function To_Moz_MethodDefinition(M, parent) {
        if (parent instanceof AST_Object) {
            return {
                type: "Property",
                computed: !(M.key instanceof AST_Symbol) || M.key instanceof AST_SymbolRef,
                kind: "init",
                method: true,
                shorthand: false,
                key: to_moz(M.key),
                value: to_moz(M.value)
            };
        }

        const key = M instanceof AST_PrivateMethod
            ? {
                type: "PrivateIdentifier",
                name: M.key.name
            }
            : to_moz(M.key);

        return {
            type: "MethodDefinition",
            kind: M.key === "constructor" ? "constructor" : "method",
            key,
            value: to_moz(M.value),
            computed: !(M.key instanceof AST_Symbol) || M.key instanceof AST_SymbolRef,
            static: M.static,
        };
    });

    def_to_moz(AST_Class, function To_Moz_Class(M) {
        var type = M instanceof AST_ClassExpression ? "ClassExpression" : "ClassDeclaration";
        return {
            type: type,
            superClass: to_moz(M.extends),
            id: M.name ? to_moz(M.name) : null,
            body: {
                type: "ClassBody",
                body: M.properties.map(to_moz)
            }
        };
    });

    def_to_moz(AST_ClassStaticBlock, function To_Moz_StaticBlock(M) {
        return {
            type: "StaticBlock",
            body: M.body.map(to_moz),
        };
    });

    def_to_moz(AST_NewTarget, function To_Moz_MetaProperty() {
        return {
            type: "MetaProperty",
            meta: {
                type: "Identifier",
                name: "new"
            },
            property: {
                type: "Identifier",
                name: "target"
            }
        };
    });

    def_to_moz(AST_Symbol, function To_Moz_Identifier(M, parent) {
        if (
            (M instanceof AST_SymbolMethod && parent.quote) ||
            ((
                M instanceof AST_SymbolImportForeign ||
                M instanceof AST_SymbolExportForeign ||
                M instanceof AST_SymbolExport
                ) && M.quote)
         ) {
            return {
                type: "Literal",
                value: M.name
            };
        }
        var def = M.definition();
        return {
            type: "Identifier",
            name: def ? def.mangled_name || def.name : M.name
        };
    });

    def_to_moz(AST_RegExp, function To_Moz_RegExpLiteral(M) {
        const pattern = M.value.source;
        const flags = M.value.flags;
        return {
            type: "Literal",
            value: null,
            raw: M.print_to_string(),
            regex: { pattern, flags }
        };
    });

    def_to_moz(AST_Constant, function To_Moz_Literal(M) {
        var value = M.value;
        return {
            type: "Literal",
            value: value,
            raw: M.raw || M.print_to_string()
        };
    });

    def_to_moz(AST_Atom, function To_Moz_Atom(M) {
        return {
            type: "Identifier",
            name: String(M.value)
        };
    });

    def_to_moz(AST_BigInt, M => ({
        type: "BigIntLiteral",
        value: M.value
    }));

    AST_Boolean.DEFMETHOD("to_mozilla_ast", AST_Constant.prototype.to_mozilla_ast);
    AST_Null.DEFMETHOD("to_mozilla_ast", AST_Constant.prototype.to_mozilla_ast);
    AST_Hole.DEFMETHOD("to_mozilla_ast", function To_Moz_ArrayHole() { return null; });

    AST_Block.DEFMETHOD("to_mozilla_ast", AST_BlockStatement.prototype.to_mozilla_ast);
    AST_Lambda.DEFMETHOD("to_mozilla_ast", AST_Function.prototype.to_mozilla_ast);

    /* -----[ tools ]----- */

    function my_start_token(moznode) {
        var loc = moznode.loc, start = loc && loc.start;
        var range = moznode.range;
        return new AST_Token(
            "",
            "",
            start && start.line || 0,
            start && start.column || 0,
            range ? range [0] : moznode.start,
            false,
            [],
            [],
            loc && loc.source,
        );
    }

    function my_end_token(moznode) {
        var loc = moznode.loc, end = loc && loc.end;
        var range = moznode.range;
        return new AST_Token(
            "",
            "",
            end && end.line || 0,
            end && end.column || 0,
            range ? range [0] : moznode.end,
            false,
            [],
            [],
            loc && loc.source,
        );
    }

    var FROM_MOZ_STACK = null;

    function from_moz(node) {
        FROM_MOZ_STACK.push(node);
        var ret = node != null ? MOZ_TO_ME[node.type](node) : null;
        FROM_MOZ_STACK.pop();
        return ret;
    }

    AST_Node.from_mozilla_ast = function(node) {
        var save_stack = FROM_MOZ_STACK;
        FROM_MOZ_STACK = [];
        var ast = from_moz(node);
        FROM_MOZ_STACK = save_stack;
        return ast;
    };

    function set_moz_loc(mynode, moznode) {
        var start = mynode.start;
        var end = mynode.end;
        if (!(start && end)) {
            return moznode;
        }
        if (start.pos != null && end.endpos != null) {
            moznode.range = [start.pos, end.endpos];
        }
        if (start.line) {
            moznode.loc = {
                start: {line: start.line, column: start.col},
                end: end.endline ? {line: end.endline, column: end.endcol} : null
            };
            if (start.file) {
                moznode.loc.source = start.file;
            }
        }
        return moznode;
    }

    function def_to_moz(mytype, handler) {
        mytype.DEFMETHOD("to_mozilla_ast", function(parent) {
            return set_moz_loc(this, handler(this, parent));
        });
    }

    var TO_MOZ_STACK = null;

    function to_moz(node) {
        if (TO_MOZ_STACK === null) { TO_MOZ_STACK = []; }
        TO_MOZ_STACK.push(node);
        var ast = node != null ? node.to_mozilla_ast(TO_MOZ_STACK[TO_MOZ_STACK.length - 2]) : null;
        TO_MOZ_STACK.pop();
        if (TO_MOZ_STACK.length === 0) { TO_MOZ_STACK = null; }
        return ast;
    }

    function to_moz_in_destructuring() {
        var i = TO_MOZ_STACK.length;
        while (i--) {
            if (TO_MOZ_STACK[i] instanceof AST_Destructuring) {
                return true;
            }
        }
        return false;
    }

    function to_moz_block(node) {
        return {
            type: "BlockStatement",
            body: node.body.map(to_moz)
        };
    }

    function to_moz_scope(type, node) {
        var body = node.body.map(to_moz);
        if (node.body[0] instanceof AST_SimpleStatement && node.body[0].body instanceof AST_String) {
            body.unshift(to_moz(new AST_EmptyStatement(node.body[0])));
        }
        return {
            type: type,
            body: body
        };
    }
})();

// return true if the node at the top of the stack (that means the
// innermost node in the current output) is lexically the first in
// a statement.
function first_in_statement(stack) {
    let node = stack.parent(-1);
    for (let i = 0, p; p = stack.parent(i); i++) {
        if (p instanceof AST_Statement && p.body === node)
            return true;
        if ((p instanceof AST_Sequence && p.expressions[0] === node) ||
            (p.TYPE === "Call" && p.expression === node) ||
            (p instanceof AST_PrefixedTemplateString && p.prefix === node) ||
            (p instanceof AST_Dot && p.expression === node) ||
            (p instanceof AST_Sub && p.expression === node) ||
            (p instanceof AST_Chain && p.expression === node) ||
            (p instanceof AST_Conditional && p.condition === node) ||
            (p instanceof AST_Binary && p.left === node) ||
            (p instanceof AST_UnaryPostfix && p.expression === node)
        ) {
            node = p;
        } else {
            return false;
        }
    }
}

// Returns whether the leftmost item in the expression is an object
function left_is_object(node) {
    if (node instanceof AST_Object) return true;
    if (node instanceof AST_Sequence) return left_is_object(node.expressions[0]);
    if (node.TYPE === "Call") return left_is_object(node.expression);
    if (node instanceof AST_PrefixedTemplateString) return left_is_object(node.prefix);
    if (node instanceof AST_Dot || node instanceof AST_Sub) return left_is_object(node.expression);
    if (node instanceof AST_Chain) return left_is_object(node.expression);
    if (node instanceof AST_Conditional) return left_is_object(node.condition);
    if (node instanceof AST_Binary) return left_is_object(node.left);
    if (node instanceof AST_UnaryPostfix) return left_is_object(node.expression);
    return false;
}

/***********************************************************************

  A JavaScript tokenizer / parser / beautifier / compressor.
  https://github.com/mishoo/UglifyJS2

  -------------------------------- (C) ---------------------------------

                           Author: Mihai Bazon
                         <mihai.bazon@gmail.com>
                       http://mihai.bazon.net/blog

  Distributed under the BSD license:

    Copyright 2012 (c) Mihai Bazon <mihai.bazon@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        * Redistributions of source code must retain the above
          copyright notice, this list of conditions and the following
          disclaimer.

        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials
          provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

 ***********************************************************************/

const EXPECT_DIRECTIVE = /^$|[;{][\s\n]*$/;
const CODE_LINE_BREAK = 10;
const CODE_SPACE = 32;

const r_annotation = /[@#]__(PURE|INLINE|NOINLINE)__/g;

function is_some_comments(comment) {
    // multiline comment
    return (
        (comment.type === "comment2" || comment.type === "comment1")
        && /@preserve|@copyright|@lic|@cc_on|^\**!/i.test(comment.value)
    );
}

class Rope {
    constructor() {
        this.committed = "";
        this.current = "";
    }

    append(str) {
        this.current += str;
    }

    insertAt(char, index) {
        const { committed, current } = this;
        if (index < committed.length) {
            this.committed = committed.slice(0, index) + char + committed.slice(index);
        } else if (index === committed.length) {
            this.committed += char;
        } else {
            index -= committed.length;
            this.committed += current.slice(0, index) + char;
            this.current = current.slice(index);
        }
    }

    charAt(index) {
        const { committed } = this;
        if (index < committed.length) return committed[index];
        return this.current[index - committed.length];
    }

    curLength() {
        return this.current.length;
    }

    length() {
        return this.committed.length + this.current.length;
    }

    toString() {
        return this.committed + this.current;
    }
}

function OutputStream(options) {

    var readonly = !options;
    options = defaults(options, {
        ascii_only           : false,
        beautify             : false,
        braces               : false,
        comments             : "some",
        ecma                 : 5,
        ie8                  : false,
        indent_level         : 4,
        indent_start         : 0,
        inline_script        : true,
        keep_numbers         : false,
        keep_quoted_props    : false,
        max_line_len         : false,
        preamble             : null,
        preserve_annotations : false,
        quote_keys           : false,
        quote_style          : 0,
        safari10             : false,
        semicolons           : true,
        shebang              : true,
        shorthand            : undefined,
        source_map           : null,
        webkit               : false,
        width                : 80,
        wrap_iife            : false,
        wrap_func_args       : true,

        _destroy_ast         : false
    }, true);

    if (options.shorthand === undefined)
        options.shorthand = options.ecma > 5;

    // Convert comment option to RegExp if necessary and set up comments filter
    var comment_filter = return_false; // Default case, throw all comments away
    if (options.comments) {
        let comments = options.comments;
        if (typeof options.comments === "string" && /^\/.*\/[a-zA-Z]*$/.test(options.comments)) {
            var regex_pos = options.comments.lastIndexOf("/");
            comments = new RegExp(
                options.comments.substr(1, regex_pos - 1),
                options.comments.substr(regex_pos + 1)
            );
        }
        if (comments instanceof RegExp) {
            comment_filter = function(comment) {
                return comment.type != "comment5" && comments.test(comment.value);
            };
        } else if (typeof comments === "function") {
            comment_filter = function(comment) {
                return comment.type != "comment5" && comments(this, comment);
            };
        } else if (comments === "some") {
            comment_filter = is_some_comments;
        } else { // NOTE includes "all" option
            comment_filter = return_true;
        }
    }

    var indentation = 0;
    var current_col = 0;
    var current_line = 1;
    var current_pos = 0;
    var OUTPUT = new Rope();
    let printed_comments = new Set();

    var to_utf8 = options.ascii_only ? function(str, identifier = false, regexp = false) {
        if (options.ecma >= 2015 && !options.safari10 && !regexp) {
            str = str.replace(/[\ud800-\udbff][\udc00-\udfff]/g, function(ch) {
                var code = get_full_char_code(ch, 0).toString(16);
                return "\\u{" + code + "}";
            });
        }
        return str.replace(/[\u0000-\u001f\u007f-\uffff]/g, function(ch) {
            var code = ch.charCodeAt(0).toString(16);
            if (code.length <= 2 && !identifier) {
                while (code.length < 2) code = "0" + code;
                return "\\x" + code;
            } else {
                while (code.length < 4) code = "0" + code;
                return "\\u" + code;
            }
        });
    } : function(str) {
        return str.replace(/[\ud800-\udbff][\udc00-\udfff]|([\ud800-\udbff]|[\udc00-\udfff])/g, function(match, lone) {
            if (lone) {
                return "\\u" + lone.charCodeAt(0).toString(16);
            }
            return match;
        });
    };

    function make_string(str, quote) {
        var dq = 0, sq = 0;
        str = str.replace(/[\\\b\f\n\r\v\t\x22\x27\u2028\u2029\0\ufeff]/g,
          function(s, i) {
            switch (s) {
              case '"': ++dq; return '"';
              case "'": ++sq; return "'";
              case "\\": return "\\\\";
              case "\n": return "\\n";
              case "\r": return "\\r";
              case "\t": return "\\t";
              case "\b": return "\\b";
              case "\f": return "\\f";
              case "\x0B": return options.ie8 ? "\\x0B" : "\\v";
              case "\u2028": return "\\u2028";
              case "\u2029": return "\\u2029";
              case "\ufeff": return "\\ufeff";
              case "\0":
                  return /[0-9]/.test(get_full_char(str, i+1)) ? "\\x00" : "\\0";
            }
            return s;
        });
        function quote_single() {
            return "'" + str.replace(/\x27/g, "\\'") + "'";
        }
        function quote_double() {
            return '"' + str.replace(/\x22/g, '\\"') + '"';
        }
        function quote_template() {
            return "`" + str.replace(/`/g, "\\`") + "`";
        }
        str = to_utf8(str);
        if (quote === "`") return quote_template();
        switch (options.quote_style) {
          case 1:
            return quote_single();
          case 2:
            return quote_double();
          case 3:
            return quote == "'" ? quote_single() : quote_double();
          default:
            return dq > sq ? quote_single() : quote_double();
        }
    }

    function encode_string(str, quote) {
        var ret = make_string(str, quote);
        if (options.inline_script) {
            ret = ret.replace(/<\x2f(script)([>\/\t\n\f\r ])/gi, "<\\/$1$2");
            ret = ret.replace(/\x3c!--/g, "\\x3c!--");
            ret = ret.replace(/--\x3e/g, "--\\x3e");
        }
        return ret;
    }

    function make_name(name) {
        name = name.toString();
        name = to_utf8(name, true);
        return name;
    }

    function make_indent(back) {
        return " ".repeat(options.indent_start + indentation - back * options.indent_level);
    }

    /* -----[ beautification/minification ]----- */

    var has_parens = false;
    var might_need_space = false;
    var might_need_semicolon = false;
    var might_add_newline = 0;
    var need_newline_indented = false;
    var need_space = false;
    var newline_insert = -1;
    var last = "";
    var mapping_token, mapping_name, mappings = options.source_map && [];

    var do_add_mapping = mappings ? function() {
        mappings.forEach(function(mapping) {
            try {
                let { name, token } = mapping;
                if (token.type == "name" || token.type === "privatename") {
                    name = token.value;
                } else if (name instanceof AST_Symbol) {
                    name = token.type === "string" ? token.value : name.name;
                }
                options.source_map.add(
                    mapping.token.file,
                    mapping.line, mapping.col,
                    mapping.token.line, mapping.token.col,
                    is_basic_identifier_string(name) ? name : undefined
                );
            } catch(ex) {
                // Ignore bad mapping
            }
        });
        mappings = [];
    } : noop;

    var ensure_line_len = options.max_line_len ? function() {
        if (current_col > options.max_line_len) {
            if (might_add_newline) {
                OUTPUT.insertAt("\n", might_add_newline);
                const curLength = OUTPUT.curLength();
                if (mappings) {
                    var delta = curLength - current_col;
                    mappings.forEach(function(mapping) {
                        mapping.line++;
                        mapping.col += delta;
                    });
                }
                current_line++;
                current_pos++;
                current_col = curLength;
            }
        }
        if (might_add_newline) {
            might_add_newline = 0;
            do_add_mapping();
        }
    } : noop;

    var requireSemicolonChars = makePredicate("( [ + * / - , . `");

    function print(str) {
        str = String(str);
        var ch = get_full_char(str, 0);
        if (need_newline_indented && ch) {
            need_newline_indented = false;
            if (ch !== "\n") {
                print("\n");
                indent();
            }
        }
        if (need_space && ch) {
            need_space = false;
            if (!/[\s;})]/.test(ch)) {
                space();
            }
        }
        newline_insert = -1;
        var prev = last.charAt(last.length - 1);
        if (might_need_semicolon) {
            might_need_semicolon = false;

            if (prev === ":" && ch === "}" || (!ch || !";}".includes(ch)) && prev !== ";") {
                if (options.semicolons || requireSemicolonChars.has(ch)) {
                    OUTPUT.append(";");
                    current_col++;
                    current_pos++;
                } else {
                    ensure_line_len();
                    if (current_col > 0) {
                        OUTPUT.append("\n");
                        current_pos++;
                        current_line++;
                        current_col = 0;
                    }

                    if (/^\s+$/.test(str)) {
                        // reset the semicolon flag, since we didn't print one
                        // now and might still have to later
                        might_need_semicolon = true;
                    }
                }

                if (!options.beautify)
                    might_need_space = false;
            }
        }

        if (might_need_space) {
            if ((is_identifier_char(prev)
                    && (is_identifier_char(ch) || ch == "\\"))
                || (ch == "/" && ch == prev)
                || ((ch == "+" || ch == "-") && ch == last)
            ) {
                OUTPUT.append(" ");
                current_col++;
                current_pos++;
            }
            might_need_space = false;
        }

        if (mapping_token) {
            mappings.push({
                token: mapping_token,
                name: mapping_name,
                line: current_line,
                col: current_col
            });
            mapping_token = false;
            if (!might_add_newline) do_add_mapping();
        }

        OUTPUT.append(str);
        has_parens = str[str.length - 1] == "(";
        current_pos += str.length;
        var a = str.split(/\r?\n/), n = a.length - 1;
        current_line += n;
        current_col += a[0].length;
        if (n > 0) {
            ensure_line_len();
            current_col = a[n].length;
        }
        last = str;
    }

    var star = function() {
        print("*");
    };

    var space = options.beautify ? function() {
        print(" ");
    } : function() {
        might_need_space = true;
    };

    var indent = options.beautify ? function(half) {
        if (options.beautify) {
            print(make_indent(half ? 0.5 : 0));
        }
    } : noop;

    var with_indent = options.beautify ? function(col, cont) {
        if (col === true) col = next_indent();
        var save_indentation = indentation;
        indentation = col;
        var ret = cont();
        indentation = save_indentation;
        return ret;
    } : function(col, cont) { return cont(); };

    var newline = options.beautify ? function() {
        if (newline_insert < 0) return print("\n");
        if (OUTPUT.charAt(newline_insert) != "\n") {
            OUTPUT.insertAt("\n", newline_insert);
            current_pos++;
            current_line++;
        }
        newline_insert++;
    } : options.max_line_len ? function() {
        ensure_line_len();
        might_add_newline = OUTPUT.length();
    } : noop;

    var semicolon = options.beautify ? function() {
        print(";");
    } : function() {
        might_need_semicolon = true;
    };

    function force_semicolon() {
        might_need_semicolon = false;
        print(";");
    }

    function next_indent() {
        return indentation + options.indent_level;
    }

    function with_block(cont) {
        var ret;
        print("{");
        newline();
        with_indent(next_indent(), function() {
            ret = cont();
        });
        indent();
        print("}");
        return ret;
    }

    function with_parens(cont) {
        print("(");
        //XXX: still nice to have that for argument lists
        //var ret = with_indent(current_col, cont);
        var ret = cont();
        print(")");
        return ret;
    }

    function with_square(cont) {
        print("[");
        //var ret = with_indent(current_col, cont);
        var ret = cont();
        print("]");
        return ret;
    }

    function comma() {
        print(",");
        space();
    }

    function colon() {
        print(":");
        space();
    }

    var add_mapping = mappings ? function(token, name) {
        mapping_token = token;
        mapping_name = name;
    } : noop;

    function get() {
        if (might_add_newline) {
            ensure_line_len();
        }
        return OUTPUT.toString();
    }

    function has_nlb() {
        const output = OUTPUT.toString();
        let n = output.length - 1;
        while (n >= 0) {
            const code = output.charCodeAt(n);
            if (code === CODE_LINE_BREAK) {
                return true;
            }

            if (code !== CODE_SPACE) {
                return false;
            }
            n--;
        }
        return true;
    }

    function filter_comment(comment) {
        if (!options.preserve_annotations) {
            comment = comment.replace(r_annotation, " ");
        }
        if (/^\s*$/.test(comment)) {
            return "";
        }
        return comment.replace(/(<\s*\/\s*)(script)/i, "<\\/$2");
    }

    function prepend_comments(node) {
        var self = this;
        var start = node.start;
        if (!start) return;
        var printed_comments = self.printed_comments;

        // There cannot be a newline between return/yield and its value.
        const keyword_with_value = 
            node instanceof AST_Exit && node.value
            || (node instanceof AST_Await || node instanceof AST_Yield)
                && node.expression;

        if (
            start.comments_before
            && printed_comments.has(start.comments_before)
        ) {
            if (keyword_with_value) {
                start.comments_before = [];
            } else {
                return;
            }
        }

        var comments = start.comments_before;
        if (!comments) {
            comments = start.comments_before = [];
        }
        printed_comments.add(comments);

        if (keyword_with_value) {
            var tw = new TreeWalker(function(node) {
                var parent = tw.parent();
                if (parent instanceof AST_Exit
                    || parent instanceof AST_Await
                    || parent instanceof AST_Yield
                    || parent instanceof AST_Binary && parent.left === node
                    || parent.TYPE == "Call" && parent.expression === node
                    || parent instanceof AST_Conditional && parent.condition === node
                    || parent instanceof AST_Dot && parent.expression === node
                    || parent instanceof AST_Sequence && parent.expressions[0] === node
                    || parent instanceof AST_Sub && parent.expression === node
                    || parent instanceof AST_UnaryPostfix) {
                    if (!node.start) return;
                    var text = node.start.comments_before;
                    if (text && !printed_comments.has(text)) {
                        printed_comments.add(text);
                        comments = comments.concat(text);
                    }
                } else {
                    return true;
                }
            });
            tw.push(node);
            keyword_with_value.walk(tw);
        }

        if (current_pos == 0) {
            if (comments.length > 0 && options.shebang && comments[0].type === "comment5"
                && !printed_comments.has(comments[0])) {
                print("#!" + comments.shift().value + "\n");
                indent();
            }
            var preamble = options.preamble;
            if (preamble) {
                print(preamble.replace(/\r\n?|[\n\u2028\u2029]|\s*$/g, "\n"));
            }
        }

        comments = comments.filter(comment_filter, node).filter(c => !printed_comments.has(c));
        if (comments.length == 0) return;
        var last_nlb = has_nlb();
        comments.forEach(function(c, i) {
            printed_comments.add(c);
            if (!last_nlb) {
                if (c.nlb) {
                    print("\n");
                    indent();
                    last_nlb = true;
                } else if (i > 0) {
                    space();
                }
            }

            if (/comment[134]/.test(c.type)) {
                var value = filter_comment(c.value);
                if (value) {
                    print("//" + value + "\n");
                    indent();
                }
                last_nlb = true;
            } else if (c.type == "comment2") {
                var value = filter_comment(c.value);
                if (value) {
                    print("/*" + value + "*/");
                }
                last_nlb = false;
            }
        });
        if (!last_nlb) {
            if (start.nlb) {
                print("\n");
                indent();
            } else {
                space();
            }
        }
    }

    function append_comments(node, tail) {
        var self = this;
        var token = node.end;
        if (!token) return;
        var printed_comments = self.printed_comments;
        var comments = token[tail ? "comments_before" : "comments_after"];
        if (!comments || printed_comments.has(comments)) return;
        if (!(node instanceof AST_Statement || comments.every((c) =>
            !/comment[134]/.test(c.type)
        ))) return;
        printed_comments.add(comments);
        var insert = OUTPUT.length();
        comments.filter(comment_filter, node).forEach(function(c, i) {
            if (printed_comments.has(c)) return;
            printed_comments.add(c);
            need_space = false;
            if (need_newline_indented) {
                print("\n");
                indent();
                need_newline_indented = false;
            } else if (c.nlb && (i > 0 || !has_nlb())) {
                print("\n");
                indent();
            } else if (i > 0 || !tail) {
                space();
            }
            if (/comment[134]/.test(c.type)) {
                const value = filter_comment(c.value);
                if (value) {
                    print("//" + value);
                }
                need_newline_indented = true;
            } else if (c.type == "comment2") {
                const value = filter_comment(c.value);
                if (value) {
                    print("/*" + value + "*/");
                }
                need_space = true;
            }
        });
        if (OUTPUT.length() > insert) newline_insert = insert;
    }

    /**
     * When output.option("_destroy_ast") is enabled, destroy the function.
     * Call this after printing it.
     */
    const gc_scope =
      options["_destroy_ast"]
        ? function gc_scope(scope) {
            scope.body.length = 0;
            scope.argnames.length = 0;
        }
        : noop;

    var stack = [];
    return {
        get             : get,
        toString        : get,
        indent          : indent,
        in_directive    : false,
        use_asm         : null,
        active_scope    : null,
        indentation     : function() { return indentation; },
        current_width   : function() { return current_col - indentation; },
        should_break    : function() { return options.width && this.current_width() >= options.width; },
        has_parens      : function() { return has_parens; },
        newline         : newline,
        print           : print,
        star            : star,
        space           : space,
        comma           : comma,
        colon           : colon,
        last            : function() { return last; },
        semicolon       : semicolon,
        force_semicolon : force_semicolon,
        to_utf8         : to_utf8,
        print_name      : function(name) { print(make_name(name)); },
        print_string    : function(str, quote, escape_directive) {
            var encoded = encode_string(str, quote);
            if (escape_directive === true && !encoded.includes("\\")) {
                // Insert semicolons to break directive prologue
                if (!EXPECT_DIRECTIVE.test(OUTPUT.toString())) {
                    force_semicolon();
                }
                force_semicolon();
            }
            print(encoded);
        },
        print_template_string_chars: function(str) {
            var encoded = encode_string(str, "`").replace(/\${/g, "\\${");
            return print(encoded.substr(1, encoded.length - 2));
        },
        encode_string   : encode_string,
        next_indent     : next_indent,
        with_indent     : with_indent,
        with_block      : with_block,
        with_parens     : with_parens,
        with_square     : with_square,
        add_mapping     : add_mapping,
        option          : function(opt) { return options[opt]; },
        gc_scope,
        printed_comments: printed_comments,
        prepend_comments: readonly ? noop : prepend_comments,
        append_comments : readonly || comment_filter === return_false ? noop : append_comments,
        line            : function() { return current_line; },
        col             : function() { return current_col; },
        pos             : function() { return current_pos; },
        push_node       : function(node) { stack.push(node); },
        pop_node        : function() { return stack.pop(); },
        parent          : function(n) {
            return stack[stack.length - 2 - (n || 0)];
        }
    };

}

/* -----[ code generators ]----- */

(function() {

    /* -----[ utils ]----- */

    function DEFPRINT(nodetype, generator) {
        nodetype.DEFMETHOD("_codegen", generator);
    }

    AST_Node.DEFMETHOD("print", function(output, force_parens) {
        var self = this, generator = self._codegen;
        if (self instanceof AST_Scope) {
            output.active_scope = self;
        } else if (!output.use_asm && self instanceof AST_Directive && self.value == "use asm") {
            output.use_asm = output.active_scope;
        }
        function doit() {
            output.prepend_comments(self);
            self.add_source_map(output);
            generator(self, output);
            output.append_comments(self);
        }
        output.push_node(self);
        if (force_parens || self.needs_parens(output)) {
            output.with_parens(doit);
        } else {
            doit();
        }
        output.pop_node();
        if (self === output.use_asm) {
            output.use_asm = null;
        }
    });
    AST_Node.DEFMETHOD("_print", AST_Node.prototype.print);

    AST_Node.DEFMETHOD("print_to_string", function(options) {
        var output = OutputStream(options);
        this.print(output);
        return output.get();
    });

    /* -----[ PARENTHESES ]----- */

    function PARENS(nodetype, func) {
        if (Array.isArray(nodetype)) {
            nodetype.forEach(function(nodetype) {
                PARENS(nodetype, func);
            });
        } else {
            nodetype.DEFMETHOD("needs_parens", func);
        }
    }

    PARENS(AST_Node, return_false);

    // a function expression needs parens around it when it's provably
    // the first token to appear in a statement.
    PARENS(AST_Function, function(output) {
        if (!output.has_parens() && first_in_statement(output)) {
            return true;
        }

        if (output.option("webkit")) {
            var p = output.parent();
            if (p instanceof AST_PropAccess && p.expression === this) {
                return true;
            }
        }

        if (output.option("wrap_iife")) {
            var p = output.parent();
            if (p instanceof AST_Call && p.expression === this) {
                return true;
            }
        }

        if (output.option("wrap_func_args")) {
            var p = output.parent();
            if (p instanceof AST_Call && p.args.includes(this)) {
                return true;
            }
        }

        return false;
    });

    PARENS(AST_Arrow, function(output) {
        var p = output.parent();

        if (
            output.option("wrap_func_args")
            && p instanceof AST_Call
            && p.args.includes(this)
        ) {
            return true;
        }
        return p instanceof AST_PropAccess && p.expression === this;
    });

    // same goes for an object literal (as in AST_Function), because
    // otherwise {...} would be interpreted as a block of code.
    PARENS(AST_Object, function(output) {
        return !output.has_parens() && first_in_statement(output);
    });

    PARENS(AST_ClassExpression, first_in_statement);

    PARENS(AST_Unary, function(output) {
        var p = output.parent();
        return p instanceof AST_PropAccess && p.expression === this
            || p instanceof AST_Call && p.expression === this
            || p instanceof AST_Binary
                && p.operator === "**"
                && this instanceof AST_UnaryPrefix
                && p.left === this
                && this.operator !== "++"
                && this.operator !== "--";
    });

    PARENS(AST_Await, function(output) {
        var p = output.parent();
        return p instanceof AST_PropAccess && p.expression === this
            || p instanceof AST_Call && p.expression === this
            || p instanceof AST_Binary && p.operator === "**" && p.left === this
            || output.option("safari10") && p instanceof AST_UnaryPrefix;
    });

    PARENS(AST_Sequence, function(output) {
        var p = output.parent();
        return p instanceof AST_Call                          // (foo, bar)() or foo(1, (2, 3), 4)
            || p instanceof AST_Unary                         // !(foo, bar, baz)
            || p instanceof AST_Binary                        // 1 + (2, 3) + 4 ==> 8
            || p instanceof AST_VarDef                        // var a = (1, 2), b = a + a; ==> b == 4
            || p instanceof AST_PropAccess                    // (1, {foo:2}).foo or (1, {foo:2})["foo"] ==> 2
            || p instanceof AST_Array                         // [ 1, (2, 3), 4 ] ==> [ 1, 3, 4 ]
            || p instanceof AST_ObjectProperty                // { foo: (1, 2) }.foo ==> 2
            || p instanceof AST_Conditional                   /* (false, true) ? (a = 10, b = 20) : (c = 30)
                                                               * ==> 20 (side effect, set a := 10 and b := 20) */
            || p instanceof AST_Arrow                         // x => (x, x)
            || p instanceof AST_DefaultAssign                 // x => (x = (0, function(){}))
            || p instanceof AST_Expansion                     // [...(a, b)]
            || p instanceof AST_ForOf && this === p.object    // for (e of (foo, bar)) {}
            || p instanceof AST_Yield                         // yield (foo, bar)
            || p instanceof AST_Export                        // export default (foo, bar)
        ;
    });

    PARENS(AST_Binary, function(output) {
        var p = output.parent();
        // (foo && bar)()
        if (p instanceof AST_Call && p.expression === this)
            return true;
        // typeof (foo && bar)
        if (p instanceof AST_Unary)
            return true;
        // (foo && bar)["prop"], (foo && bar).prop
        if (p instanceof AST_PropAccess && p.expression === this)
            return true;
        // this deals with precedence: 3 * (2 + 1)
        if (p instanceof AST_Binary) {
            const po = p.operator;
            const so = this.operator;

            if (so === "??" && (po === "||" || po === "&&")) {
                return true;
            }

            if (po === "??" && (so === "||" || so === "&&")) {
                return true;
            }

            const pp = PRECEDENCE[po];
            const sp = PRECEDENCE[so];
            if (pp > sp
                || (pp == sp
                    && (this === p.right || po == "**"))) {
                return true;
            }
        }
    });

    PARENS(AST_Yield, function(output) {
        var p = output.parent();
        // (yield 1) + (yield 2)
        // a = yield 3
        if (p instanceof AST_Binary && p.operator !== "=")
            return true;
        // (yield 1)()
        // new (yield 1)()
        if (p instanceof AST_Call && p.expression === this)
            return true;
        // (yield 1) ? yield 2 : yield 3
        if (p instanceof AST_Conditional && p.condition === this)
            return true;
        // -(yield 4)
        if (p instanceof AST_Unary)
            return true;
        // (yield x).foo
        // (yield x)['foo']
        if (p instanceof AST_PropAccess && p.expression === this)
            return true;
    });

    PARENS(AST_Chain, function(output) {
        var p = output.parent();
        if (!(p instanceof AST_Call || p instanceof AST_PropAccess)) return false;
        return p.expression === this;
    });

    PARENS(AST_PropAccess, function(output) {
        var p = output.parent();
        if (p instanceof AST_New && p.expression === this) {
            // i.e. new (foo.bar().baz)
            //
            // if there's one call into this subtree, then we need
            // parens around it too, otherwise the call will be
            // interpreted as passing the arguments to the upper New
            // expression.
            return walk(this, node => {
                if (node instanceof AST_Scope) return true;
                if (node instanceof AST_Call) {
                    return walk_abort;  // makes walk() return true.
                }
            });
        }
    });

    PARENS(AST_Call, function(output) {
        var p = output.parent(), p1;
        if (p instanceof AST_New && p.expression === this
            || p instanceof AST_Export && p.is_default && this.expression instanceof AST_Function)
            return true;

        // workaround for Safari bug.
        // https://bugs.webkit.org/show_bug.cgi?id=123506
        return this.expression instanceof AST_Function
            && p instanceof AST_PropAccess
            && p.expression === this
            && (p1 = output.parent(1)) instanceof AST_Assign
            && p1.left === p;
    });

    PARENS(AST_New, function(output) {
        var p = output.parent();
        if (this.args.length === 0
            && (p instanceof AST_PropAccess // (new Date).getTime(), (new Date)["getTime"]()
                || p instanceof AST_Call && p.expression === this
                || p instanceof AST_PrefixedTemplateString && p.prefix === this)) // (new foo)(bar)
            return true;
    });

    PARENS(AST_Number, function(output) {
        var p = output.parent();
        if (p instanceof AST_PropAccess && p.expression === this) {
            var value = this.getValue();
            if (value < 0 || /^0/.test(make_num(value))) {
                return true;
            }
        }
    });

    PARENS(AST_BigInt, function(output) {
        var p = output.parent();
        if (p instanceof AST_PropAccess && p.expression === this) {
            var value = this.getValue();
            if (value.startsWith("-")) {
                return true;
            }
        }
    });

    PARENS([ AST_Assign, AST_Conditional ], function(output) {
        var p = output.parent();
        // !(a = false) → true
        if (p instanceof AST_Unary)
            return true;
        // 1 + (a = 2) + 3 → 6, side effect setting a = 2
        if (p instanceof AST_Binary && !(p instanceof AST_Assign))
            return true;
        // (a = func)() —or— new (a = Object)()
        if (p instanceof AST_Call && p.expression === this)
            return true;
        // (a = foo) ? bar : baz
        if (p instanceof AST_Conditional && p.condition === this)
            return true;
        // (a = foo)["prop"] —or— (a = foo).prop
        if (p instanceof AST_PropAccess && p.expression === this)
            return true;
        // ({a, b} = {a: 1, b: 2}), a destructuring assignment
        if (this instanceof AST_Assign && this.left instanceof AST_Destructuring && this.left.is_array === false)
            return true;
    });

    /* -----[ PRINTERS ]----- */

    DEFPRINT(AST_Directive, function(self, output) {
        output.print_string(self.value, self.quote);
        output.semicolon();
    });

    DEFPRINT(AST_Expansion, function (self, output) {
        output.print("...");
        self.expression.print(output);
    });

    DEFPRINT(AST_Destructuring, function (self, output) {
        output.print(self.is_array ? "[" : "{");
        var len = self.names.length;
        self.names.forEach(function (name, i) {
            if (i > 0) output.comma();
            name.print(output);
            // If the final element is a hole, we need to make sure it
            // doesn't look like a trailing comma, by inserting an actual
            // trailing comma.
            if (i == len - 1 && name instanceof AST_Hole) output.comma();
        });
        output.print(self.is_array ? "]" : "}");
    });

    DEFPRINT(AST_Debugger, function(self, output) {
        output.print("debugger");
        output.semicolon();
    });

    /* -----[ statements ]----- */

    function display_body(body, is_toplevel, output, allow_directives) {
        var last = body.length - 1;
        output.in_directive = allow_directives;
        body.forEach(function(stmt, i) {
            if (output.in_directive === true && !(stmt instanceof AST_Directive ||
                stmt instanceof AST_EmptyStatement ||
                (stmt instanceof AST_SimpleStatement && stmt.body instanceof AST_String)
            )) {
                output.in_directive = false;
            }
            if (!(stmt instanceof AST_EmptyStatement)) {
                output.indent();
                stmt.print(output);
                if (!(i == last && is_toplevel)) {
                    output.newline();
                    if (is_toplevel) output.newline();
                }
            }
            if (output.in_directive === true &&
                stmt instanceof AST_SimpleStatement &&
                stmt.body instanceof AST_String
            ) {
                output.in_directive = false;
            }
        });
        output.in_directive = false;
    }

    AST_StatementWithBody.DEFMETHOD("_do_print_body", function(output) {
        print_maybe_braced_body(this.body, output);
    });

    DEFPRINT(AST_Statement, function(self, output) {
        self.body.print(output);
        output.semicolon();
    });
    DEFPRINT(AST_Toplevel, function(self, output) {
        display_body(self.body, true, output, true);
        output.print("");
    });
    DEFPRINT(AST_LabeledStatement, function(self, output) {
        self.label.print(output);
        output.colon();
        self.body.print(output);
    });
    DEFPRINT(AST_SimpleStatement, function(self, output) {
        self.body.print(output);
        output.semicolon();
    });
// XXX Emscripten localmod: Add a node type for a parenthesized expression so that we can retain
// Closure annotations that need a form "/**annotation*/(expression)"
    DEFPRINT(AST_ParenthesizedExpression, function(self, output) {
        output.print('(');
        self.body.print(output);
        output.print(')');
    });
// XXX End Emscripten localmod
    function print_braced_empty(self, output) {
        output.print("{");
        output.with_indent(output.next_indent(), function() {
            output.append_comments(self, true);
        });
        output.add_mapping(self.end);
        output.print("}");
    }
    function print_braced(self, output, allow_directives) {
        if (self.body.length > 0) {
            output.with_block(function() {
                display_body(self.body, false, output, allow_directives);
                output.add_mapping(self.end);
            });
        } else print_braced_empty(self, output);
    }
    DEFPRINT(AST_BlockStatement, function(self, output) {
        print_braced(self, output);
    });
    DEFPRINT(AST_EmptyStatement, function(self, output) {
        output.semicolon();
    });
    DEFPRINT(AST_Do, function(self, output) {
        output.print("do");
        output.space();
        make_block(self.body, output);
        output.space();
        output.print("while");
        output.space();
        output.with_parens(function() {
            self.condition.print(output);
        });
        output.semicolon();
    });
    DEFPRINT(AST_While, function(self, output) {
        output.print("while");
        output.space();
        output.with_parens(function() {
            self.condition.print(output);
        });
        output.space();
        self._do_print_body(output);
    });
    DEFPRINT(AST_For, function(self, output) {
        output.print("for");
        output.space();
        output.with_parens(function() {
            if (self.init) {
                if (self.init instanceof AST_Definitions) {
                    self.init.print(output);
                } else {
                    parenthesize_for_noin(self.init, output, true);
                }
                output.print(";");
                output.space();
            } else {
                output.print(";");
            }
            if (self.condition) {
                self.condition.print(output);
                output.print(";");
                output.space();
            } else {
                output.print(";");
            }
            if (self.step) {
                self.step.print(output);
            }
        });
        output.space();
        self._do_print_body(output);
    });
    DEFPRINT(AST_ForIn, function(self, output) {
        output.print("for");
        if (self.await) {
            output.space();
            output.print("await");
        }
        output.space();
        output.with_parens(function() {
            self.init.print(output);
            output.space();
            output.print(self instanceof AST_ForOf ? "of" : "in");
            output.space();
            self.object.print(output);
        });
        output.space();
        self._do_print_body(output);
    });
    DEFPRINT(AST_With, function(self, output) {
        output.print("with");
        output.space();
        output.with_parens(function() {
            self.expression.print(output);
        });
        output.space();
        self._do_print_body(output);
    });

    /* -----[ functions ]----- */
    AST_Lambda.DEFMETHOD("_do_print", function(output, nokeyword) {
        var self = this;
        if (!nokeyword) {
            if (self.async) {
                output.print("async");
                output.space();
            }
            output.print("function");
            if (self.is_generator) {
                output.star();
            }
            if (self.name) {
                output.space();
            }
        }
        if (self.name instanceof AST_Symbol) {
            self.name.print(output);
        } else if (nokeyword && self.name instanceof AST_Node) {
            output.with_square(function() {
                self.name.print(output); // Computed method name
            });
        }
        output.with_parens(function() {
            self.argnames.forEach(function(arg, i) {
                if (i) output.comma();
                arg.print(output);
            });
        });
        output.space();
        print_braced(self, output, true);
    });
    DEFPRINT(AST_Lambda, function(self, output) {
        self._do_print(output);
        output.gc_scope(self);
    });

    DEFPRINT(AST_PrefixedTemplateString, function(self, output) {
        var tag = self.prefix;
        var parenthesize_tag = tag instanceof AST_Lambda
            || tag instanceof AST_Binary
            || tag instanceof AST_Conditional
            || tag instanceof AST_Sequence
            || tag instanceof AST_Unary
            || tag instanceof AST_Dot && tag.expression instanceof AST_Object;
        if (parenthesize_tag) output.print("(");
        self.prefix.print(output);
        if (parenthesize_tag) output.print(")");
        self.template_string.print(output);
    });
    DEFPRINT(AST_TemplateString, function(self, output) {
        var is_tagged = output.parent() instanceof AST_PrefixedTemplateString;

        output.print("`");
        for (var i = 0; i < self.segments.length; i++) {
            if (!(self.segments[i] instanceof AST_TemplateSegment)) {
                output.print("${");
                self.segments[i].print(output);
                output.print("}");
            } else if (is_tagged) {
                output.print(self.segments[i].raw);
            } else {
                output.print_template_string_chars(self.segments[i].value);
            }
        }
        output.print("`");
    });
    DEFPRINT(AST_TemplateSegment, function(self, output) {
        output.print_template_string_chars(self.value);
    });

    AST_Arrow.DEFMETHOD("_do_print", function(output) {
        var self = this;
        var parent = output.parent();
        var needs_parens = (parent instanceof AST_Binary && !(parent instanceof AST_Assign)) ||
            parent instanceof AST_Unary ||
            (parent instanceof AST_Call && self === parent.expression);
        if (needs_parens) { output.print("("); }
        if (self.async) {
            output.print("async");
            output.space();
        }
        if (self.argnames.length === 1 && self.argnames[0] instanceof AST_Symbol) {
            self.argnames[0].print(output);
        } else {
            output.with_parens(function() {
                self.argnames.forEach(function(arg, i) {
                    if (i) output.comma();
                    arg.print(output);
                });
            });
        }
        output.space();
        output.print("=>");
        output.space();
        const first_statement = self.body[0];
        if (
            self.body.length === 1
            && first_statement instanceof AST_Return
        ) {
            const returned = first_statement.value;
            if (!returned) {
                output.print("{}");
            } else if (left_is_object(returned)) {
                output.print("(");
                returned.print(output);
                output.print(")");
            } else {
                returned.print(output);
            }
        } else {
            print_braced(self, output);
        }
        if (needs_parens) { output.print(")"); }
        output.gc_scope(self);
    });

    /* -----[ exits ]----- */
    AST_Exit.DEFMETHOD("_do_print", function(output, kind) {
        output.print(kind);
        if (this.value) {
            output.space();
            const comments = this.value.start.comments_before;
            if (comments && comments.length && !output.printed_comments.has(comments)) {
                output.print("(");
                this.value.print(output);
                output.print(")");
            } else {
                this.value.print(output);
            }
        }
        output.semicolon();
    });
    DEFPRINT(AST_Return, function(self, output) {
        self._do_print(output, "return");
    });
    DEFPRINT(AST_Throw, function(self, output) {
        self._do_print(output, "throw");
    });

    /* -----[ yield ]----- */

    DEFPRINT(AST_Yield, function(self, output) {
        var star = self.is_star ? "*" : "";
        output.print("yield" + star);
        if (self.expression) {
            output.space();
            self.expression.print(output);
        }
    });

    DEFPRINT(AST_Await, function(self, output) {
        output.print("await");
        output.space();
        var e = self.expression;
        var parens = !(
               e instanceof AST_Call
            || e instanceof AST_SymbolRef
            || e instanceof AST_PropAccess
            || e instanceof AST_Unary
            || e instanceof AST_Constant
            || e instanceof AST_Await
            || e instanceof AST_Object
        );
        if (parens) output.print("(");
        self.expression.print(output);
        if (parens) output.print(")");
    });

    /* -----[ loop control ]----- */
    AST_LoopControl.DEFMETHOD("_do_print", function(output, kind) {
        output.print(kind);
        if (this.label) {
            output.space();
            this.label.print(output);
        }
        output.semicolon();
    });
    DEFPRINT(AST_Break, function(self, output) {
        self._do_print(output, "break");
    });
    DEFPRINT(AST_Continue, function(self, output) {
        self._do_print(output, "continue");
    });

    /* -----[ if ]----- */
    function make_then(self, output) {
        var b = self.body;
        if (output.option("braces")
            || output.option("ie8") && b instanceof AST_Do)
            return make_block(b, output);
        // The squeezer replaces "block"-s that contain only a single
        // statement with the statement itself; technically, the AST
        // is correct, but this can create problems when we output an
        // IF having an ELSE clause where the THEN clause ends in an
        // IF *without* an ELSE block (then the outer ELSE would refer
        // to the inner IF).  This function checks for this case and
        // adds the block braces if needed.
        if (!b) return output.force_semicolon();
        while (true) {
            if (b instanceof AST_If) {
                if (!b.alternative) {
                    make_block(self.body, output);
                    return;
                }
                b = b.alternative;
            } else if (b instanceof AST_StatementWithBody) {
                b = b.body;
            } else break;
        }
        print_maybe_braced_body(self.body, output);
    }
    DEFPRINT(AST_If, function(self, output) {
        output.print("if");
        output.space();
        output.with_parens(function() {
            self.condition.print(output);
        });
        output.space();
        if (self.alternative) {
            make_then(self, output);
            output.space();
            output.print("else");
            output.space();
            if (self.alternative instanceof AST_If)
                self.alternative.print(output);
            else
                print_maybe_braced_body(self.alternative, output);
        } else {
            self._do_print_body(output);
        }
    });

    /* -----[ switch ]----- */
    DEFPRINT(AST_Switch, function(self, output) {
        output.print("switch");
        output.space();
        output.with_parens(function() {
            self.expression.print(output);
        });
        output.space();
        var last = self.body.length - 1;
        if (last < 0) print_braced_empty(self, output);
        else output.with_block(function() {
            self.body.forEach(function(branch, i) {
                output.indent(true);
                branch.print(output);
                if (i < last && branch.body.length > 0)
                    output.newline();
            });
        });
    });
    AST_SwitchBranch.DEFMETHOD("_do_print_body", function(output) {
        output.newline();
        this.body.forEach(function(stmt) {
            output.indent();
            stmt.print(output);
            output.newline();
        });
    });
    DEFPRINT(AST_Default, function(self, output) {
        output.print("default:");
        self._do_print_body(output);
    });
    DEFPRINT(AST_Case, function(self, output) {
        output.print("case");
        output.space();
        self.expression.print(output);
        output.print(":");
        self._do_print_body(output);
    });

    /* -----[ exceptions ]----- */
    DEFPRINT(AST_Try, function(self, output) {
        output.print("try");
        output.space();
        self.body.print(output);
        if (self.bcatch) {
            output.space();
            self.bcatch.print(output);
        }
        if (self.bfinally) {
            output.space();
            self.bfinally.print(output);
        }
    });
    DEFPRINT(AST_TryBlock, function(self, output) {
        print_braced(self, output);
    });
    DEFPRINT(AST_Catch, function(self, output) {
        output.print("catch");
        if (self.argname) {
            output.space();
            output.with_parens(function() {
                self.argname.print(output);
            });
        }
        output.space();
        print_braced(self, output);
    });
    DEFPRINT(AST_Finally, function(self, output) {
        output.print("finally");
        output.space();
        print_braced(self, output);
    });

    /* -----[ var/const ]----- */
    AST_Definitions.DEFMETHOD("_do_print", function(output, kind) {
        output.print(kind);
        output.space();
        this.definitions.forEach(function(def, i) {
            if (i) output.comma();
            def.print(output);
        });
        var p = output.parent();
        var in_for = p instanceof AST_For || p instanceof AST_ForIn;
        var output_semicolon = !in_for || p && p.init !== this;
        if (output_semicolon)
            output.semicolon();
    });
    DEFPRINT(AST_Let, function(self, output) {
        self._do_print(output, "let");
    });
    DEFPRINT(AST_Var, function(self, output) {
        self._do_print(output, "var");
    });
    DEFPRINT(AST_Const, function(self, output) {
        self._do_print(output, "const");
    });
    DEFPRINT(AST_Import, function(self, output) {
        output.print("import");
        output.space();
        if (self.imported_name) {
            self.imported_name.print(output);
        }
        if (self.imported_name && self.imported_names) {
            output.print(",");
            output.space();
        }
        if (self.imported_names) {
            if (self.imported_names.length === 1 &&
                self.imported_names[0].foreign_name.name === "*" &&
                !self.imported_names[0].foreign_name.quote) {
                self.imported_names[0].print(output);
            } else {
                output.print("{");
                self.imported_names.forEach(function (name_import, i) {
                    output.space();
                    name_import.print(output);
                    if (i < self.imported_names.length - 1) {
                        output.print(",");
                    }
                });
                output.space();
                output.print("}");
            }
        }
        if (self.imported_name || self.imported_names) {
            output.space();
            output.print("from");
            output.space();
        }
        self.module_name.print(output);
        if (self.assert_clause) {
            output.print("assert");
            self.assert_clause.print(output);
        }
        output.semicolon();
    });
    DEFPRINT(AST_ImportMeta, function(self, output) {
        output.print("import.meta");
    });

    DEFPRINT(AST_NameMapping, function(self, output) {
        var is_import = output.parent() instanceof AST_Import;
        var definition = self.name.definition();
        var foreign_name = self.foreign_name;
        var names_are_different =
            (definition && definition.mangled_name || self.name.name) !==
            foreign_name.name;
        if (!names_are_different &&
            foreign_name.name === "*" &&
            foreign_name.quote != self.name.quote) {
                // export * as "*"
            names_are_different = true;
        }
        var foreign_name_is_name = foreign_name.quote == null;
        if (names_are_different) {
            if (is_import) {
                if (foreign_name_is_name) {
                    output.print(foreign_name.name);
                } else {
                    output.print_string(foreign_name.name, foreign_name.quote);
                }
            } else {
                if (self.name.quote == null) {
                    self.name.print(output);
                } else {
                    output.print_string(self.name.name, self.name.quote);
                }
                
            }
            output.space();
            output.print("as");
            output.space();
            if (is_import) {
                self.name.print(output);
            } else {
                if (foreign_name_is_name) {
                    output.print(foreign_name.name);
                } else {
                    output.print_string(foreign_name.name, foreign_name.quote);
                }
            }
        } else {
            if (self.name.quote == null) {
                self.name.print(output);
            } else {
                output.print_string(self.name.name, self.name.quote);
            }
        }
    });

    DEFPRINT(AST_Export, function(self, output) {
        output.print("export");
        output.space();
        if (self.is_default) {
            output.print("default");
            output.space();
        }
        if (self.exported_names) {
            if (self.exported_names.length === 1 &&
                self.exported_names[0].name.name === "*" &&
                !self.exported_names[0].name.quote) {
                    self.exported_names[0].print(output);
            } else {
                output.print("{");
                self.exported_names.forEach(function(name_export, i) {
                    output.space();
                    name_export.print(output);
                    if (i < self.exported_names.length - 1) {
                        output.print(",");
                    }
                });
                output.space();
                output.print("}");
            }
        } else if (self.exported_value) {
            self.exported_value.print(output);
        } else if (self.exported_definition) {
            self.exported_definition.print(output);
            if (self.exported_definition instanceof AST_Definitions) return;
        }
        if (self.module_name) {
            output.space();
            output.print("from");
            output.space();
            self.module_name.print(output);
        }
        if (self.assert_clause) {
            output.print("assert");
            self.assert_clause.print(output);
        }
        if (self.exported_value
                && !(self.exported_value instanceof AST_Defun ||
                    self.exported_value instanceof AST_Function ||
                    self.exported_value instanceof AST_Class)
            || self.module_name
            || self.exported_names
        ) {
            output.semicolon();
        }
    });

    function parenthesize_for_noin(node, output, noin) {
        var parens = false;
        // need to take some precautions here:
        //    https://github.com/mishoo/UglifyJS2/issues/60
        if (noin) {
            parens = walk(node, node => {
                // Don't go into scopes -- except arrow functions:
                // https://github.com/terser/terser/issues/1019#issuecomment-877642607
                if (node instanceof AST_Scope && !(node instanceof AST_Arrow)) {
                    return true;
                }
                if (
                    node instanceof AST_Binary && node.operator == "in"
                    || node instanceof AST_PrivateIn
                ) {
                    return walk_abort;  // makes walk() return true
                }
            });
        }
        node.print(output, parens);
    }

    DEFPRINT(AST_VarDef, function(self, output) {
        self.name.print(output);
        if (self.value) {
            output.space();
            output.print("=");
            output.space();
            var p = output.parent(1);
            var noin = p instanceof AST_For || p instanceof AST_ForIn;
            parenthesize_for_noin(self.value, output, noin);
        }
    });

    /* -----[ other expressions ]----- */
    DEFPRINT(AST_Call, function(self, output) {
        self.expression.print(output);
        if (self instanceof AST_New && self.args.length === 0)
            return;
        if (self.expression instanceof AST_Call || self.expression instanceof AST_Lambda) {
            output.add_mapping(self.start);
        }
        if (self.optional) output.print("?.");
        output.with_parens(function() {
            self.args.forEach(function(expr, i) {
                if (i) output.comma();
                expr.print(output);
            });
        });
    });
    DEFPRINT(AST_New, function(self, output) {
        output.print("new");
        output.space();
        AST_Call.prototype._codegen(self, output);
    });

    AST_Sequence.DEFMETHOD("_do_print", function(output) {
        this.expressions.forEach(function(node, index) {
            if (index > 0) {
                output.comma();
                if (output.should_break()) {
                    output.newline();
                    output.indent();
                }
            }
            node.print(output);
        });
    });
    DEFPRINT(AST_Sequence, function(self, output) {
        self._do_print(output);
        // var p = output.parent();
        // if (p instanceof AST_Statement) {
        //     output.with_indent(output.next_indent(), function(){
        //         self._do_print(output);
        //     });
        // } else {
        //     self._do_print(output);
        // }
    });
    DEFPRINT(AST_Dot, function(self, output) {
        var expr = self.expression;
        expr.print(output);
        var prop = self.property;
        var print_computed = ALL_RESERVED_WORDS.has(prop)
            ? output.option("ie8")
            : !is_identifier_string(
                prop,
                output.option("ecma") >= 2015 && !output.option("safari10")
            );

        if (self.optional) output.print("?.");

        if (print_computed) {
            output.print("[");
            output.add_mapping(self.end);
            output.print_string(prop);
            output.print("]");
        } else {
            if (expr instanceof AST_Number && expr.getValue() >= 0) {
                if (!/[xa-f.)]/i.test(output.last())) {
                    output.print(".");
                }
            }
            if (!self.optional) output.print(".");
            // the name after dot would be mapped about here.
            output.add_mapping(self.end);
            output.print_name(prop);
        }
    });
    DEFPRINT(AST_DotHash, function(self, output) {
        var expr = self.expression;
        expr.print(output);
        var prop = self.property;

        if (self.optional) output.print("?");
        output.print(".#");
        output.add_mapping(self.end);
        output.print_name(prop);
    });
    DEFPRINT(AST_Sub, function(self, output) {
        self.expression.print(output);
        if (self.optional) output.print("?.");
        output.print("[");
        self.property.print(output);
        output.print("]");
    });
    DEFPRINT(AST_Chain, function(self, output) {
        self.expression.print(output);
    });
    DEFPRINT(AST_UnaryPrefix, function(self, output) {
        var op = self.operator;
        output.print(op);
        if (/^[a-z]/i.test(op)
            || (/[+-]$/.test(op)
                && self.expression instanceof AST_UnaryPrefix
                && /^[+-]/.test(self.expression.operator))) {
            output.space();
        }
        self.expression.print(output);
    });
    DEFPRINT(AST_UnaryPostfix, function(self, output) {
        self.expression.print(output);
        output.print(self.operator);
    });
    DEFPRINT(AST_Binary, function(self, output) {
        var op = self.operator;
        self.left.print(output);
        if (op[0] == ">" /* ">>" ">>>" ">" ">=" */
            && self.left instanceof AST_UnaryPostfix
            && self.left.operator == "--") {
            // space is mandatory to avoid outputting -->
            output.print(" ");
        } else {
            // the space is optional depending on "beautify"
            output.space();
        }
        output.print(op);
        if ((op == "<" || op == "<<")
            && self.right instanceof AST_UnaryPrefix
            && self.right.operator == "!"
            && self.right.expression instanceof AST_UnaryPrefix
            && self.right.expression.operator == "--") {
            // space is mandatory to avoid outputting <!--
            output.print(" ");
        } else {
            // the space is optional depending on "beautify"
            output.space();
        }
        self.right.print(output);
    });
    DEFPRINT(AST_Conditional, function(self, output) {
        self.condition.print(output);
        output.space();
        output.print("?");
        output.space();
        self.consequent.print(output);
        output.space();
        output.colon();
        self.alternative.print(output);
    });

    /* -----[ literals ]----- */
    DEFPRINT(AST_Array, function(self, output) {
        output.with_square(function() {
            var a = self.elements, len = a.length;
            if (len > 0) output.space();
            a.forEach(function(exp, i) {
                if (i) output.comma();
                exp.print(output);
                // If the final element is a hole, we need to make sure it
                // doesn't look like a trailing comma, by inserting an actual
                // trailing comma.
                if (i === len - 1 && exp instanceof AST_Hole)
                  output.comma();
            });
            if (len > 0) output.space();
        });
    });
    DEFPRINT(AST_Object, function(self, output) {
        if (self.properties.length > 0) output.with_block(function() {
            self.properties.forEach(function(prop, i) {
                if (i) {
                    output.print(",");
                    output.newline();
                }
                output.indent();
                prop.print(output);
            });
            output.newline();
        });
        else print_braced_empty(self, output);
    });
    DEFPRINT(AST_Class, function(self, output) {
        output.print("class");
        output.space();
        if (self.name) {
            self.name.print(output);
            output.space();
        }
        if (self.extends) {
            var parens = (
                   !(self.extends instanceof AST_SymbolRef)
                && !(self.extends instanceof AST_PropAccess)
                && !(self.extends instanceof AST_ClassExpression)
                && !(self.extends instanceof AST_Function)
            );
            output.print("extends");
            if (parens) {
                output.print("(");
            } else {
                output.space();
            }
            self.extends.print(output);
            if (parens) {
                output.print(")");
            } else {
                output.space();
            }
        }
        if (self.properties.length > 0) output.with_block(function() {
            self.properties.forEach(function(prop, i) {
                if (i) {
                    output.newline();
                }
                output.indent();
                prop.print(output);
            });
            output.newline();
        });
        else output.print("{}");
    });
    DEFPRINT(AST_NewTarget, function(self, output) {
        output.print("new.target");
    });

    /** Prints a prop name. Returns whether it can be used as a shorthand. */
    function print_property_name(key, quote, output) {
        if (output.option("quote_keys")) {
            output.print_string(key);
            return false;
        }
        if ("" + +key == key && key >= 0) {
            if (output.option("keep_numbers")) {
                output.print(key);
                return false;
            }
            output.print(make_num(key));
            return false;
        }
        var print_string = ALL_RESERVED_WORDS.has(key)
            ? output.option("ie8")
            : (
                output.option("ecma") < 2015 || output.option("safari10")
                    ? !is_basic_identifier_string(key)
                    : !is_identifier_string(key, true)
            );
        if (print_string || (quote && output.option("keep_quoted_props"))) {
            output.print_string(key, quote);
            return false;
        }
        output.print_name(key);
        return true;
    }

    DEFPRINT(AST_ObjectKeyVal, function(self, output) {
        function get_name(self) {
            var def = self.definition();
            return def ? def.mangled_name || def.name : self.name;
        }

        const try_shorthand = output.option("shorthand") && !(self.key instanceof AST_Node);
        if (
            try_shorthand
            && self.value instanceof AST_Symbol
            && get_name(self.value) === self.key
            && !ALL_RESERVED_WORDS.has(self.key)
        ) {
            const was_shorthand = print_property_name(self.key, self.quote, output);
            if (!was_shorthand) {
                output.colon();
                self.value.print(output);
            }
        } else if (
            try_shorthand
            && self.value instanceof AST_DefaultAssign
            && self.value.left instanceof AST_Symbol
            && get_name(self.value.left) === self.key
        ) {
            const was_shorthand = print_property_name(self.key, self.quote, output);
            if (!was_shorthand) {
                output.colon();
                self.value.left.print(output);
            }
            output.space();
            output.print("=");
            output.space();
            self.value.right.print(output);
        } else {
            if (!(self.key instanceof AST_Node)) {
                print_property_name(self.key, self.quote, output);
            } else {
                output.with_square(function() {
                    self.key.print(output);
                });
            }
            output.colon();
            self.value.print(output);
        }
    });
    DEFPRINT(AST_ClassPrivateProperty, (self, output) => {
        if (self.static) {
            output.print("static");
            output.space();
        }

        output.print("#");
        
        print_property_name(self.key.name, self.quote, output);

        if (self.value) {
            output.print("=");
            self.value.print(output);
        }

        output.semicolon();
    });
    DEFPRINT(AST_ClassProperty, (self, output) => {
        if (self.static) {
            output.print("static");
            output.space();
        }

        if (self.key instanceof AST_SymbolClassProperty) {
            print_property_name(self.key.name, self.quote, output);
        } else {
            output.print("[");
            self.key.print(output);
            output.print("]");
        }

        if (self.value) {
            output.print("=");
            self.value.print(output);
        }

        output.semicolon();
    });
    AST_ObjectProperty.DEFMETHOD("_print_getter_setter", function(type, is_private, output) {
        var self = this;
        if (self.static) {
            output.print("static");
            output.space();
        }
        if (type) {
            output.print(type);
            output.space();
        }
        if (self.key instanceof AST_SymbolMethod) {
            if (is_private) output.print("#");
            print_property_name(self.key.name, self.quote, output);
        } else {
            output.with_square(function() {
                self.key.print(output);
            });
        }
        self.value._do_print(output, true);
    });
    DEFPRINT(AST_ObjectSetter, function(self, output) {
        self._print_getter_setter("set", false, output);
    });
    DEFPRINT(AST_ObjectGetter, function(self, output) {
        self._print_getter_setter("get", false, output);
    });
    DEFPRINT(AST_PrivateSetter, function(self, output) {
        self._print_getter_setter("set", true, output);
    });
    DEFPRINT(AST_PrivateGetter, function(self, output) {
        self._print_getter_setter("get", true, output);
    });
    DEFPRINT(AST_PrivateMethod, function(self, output) {
        var type;
        if (self.is_generator && self.async) {
            type = "async*";
        } else if (self.is_generator) {
            type = "*";
        } else if (self.async) {
            type = "async";
        }
        self._print_getter_setter(type, true, output);
    });
    DEFPRINT(AST_PrivateIn, function(self, output) {
        self.key.print(output);
        output.space();
        output.print("in");
        output.space();
        self.value.print(output);
    });
    DEFPRINT(AST_SymbolPrivateProperty, function(self, output) {
        output.print("#" + self.name);
    });
    DEFPRINT(AST_ConciseMethod, function(self, output) {
        var type;
        if (self.is_generator && self.async) {
            type = "async*";
        } else if (self.is_generator) {
            type = "*";
        } else if (self.async) {
            type = "async";
        }
        self._print_getter_setter(type, false, output);
    });
    DEFPRINT(AST_ClassStaticBlock, function (self, output) {
        output.print("static");
        output.space();
        print_braced(self, output);
    });
    AST_Symbol.DEFMETHOD("_do_print", function(output) {
        var def = this.definition();
        output.print_name(def ? def.mangled_name || def.name : this.name);
    });
    DEFPRINT(AST_Symbol, function (self, output) {
        self._do_print(output);
    });
    DEFPRINT(AST_Hole, noop);
    DEFPRINT(AST_This, function(self, output) {
        output.print("this");
    });
    DEFPRINT(AST_Super, function(self, output) {
        output.print("super");
    });
    DEFPRINT(AST_Constant, function(self, output) {
        output.print(self.getValue());
    });
    DEFPRINT(AST_String, function(self, output) {
        output.print_string(self.getValue(), self.quote, output.in_directive);
    });
    DEFPRINT(AST_Number, function(self, output) {
        if ((output.option("keep_numbers") || output.use_asm) && self.raw) {
            output.print(self.raw);
        } else {
            output.print(make_num(self.getValue()));
        }
    });
    DEFPRINT(AST_BigInt, function(self, output) {
        output.print(self.getValue() + "n");
    });

    const r_slash_script = /(<\s*\/\s*script)/i;
    const slash_script_replace = (_, $1) => $1.replace("/", "\\/");
    DEFPRINT(AST_RegExp, function(self, output) {
        let { source, flags } = self.getValue();
        source = regexp_source_fix(source);
        flags = flags ? sort_regexp_flags(flags) : "";
        source = source.replace(r_slash_script, slash_script_replace);

        output.print(output.to_utf8(`/${source}/${flags}`, false, true));

        const parent = output.parent();
        if (
            parent instanceof AST_Binary
            && /^\w/.test(parent.operator)
            && parent.left === self
        ) {
            output.print(" ");
        }
    });

    /** if, for, while, may or may not have braces surrounding its body */
    function print_maybe_braced_body(stat, output) {
        if (output.option("braces")) {
            make_block(stat, output);
        } else {
            if (!stat || stat instanceof AST_EmptyStatement)
                output.force_semicolon();
            else if (stat instanceof AST_Let || stat instanceof AST_Const || stat instanceof AST_Class)
                make_block(stat, output);
            else
                stat.print(output);
        }
    }

    function best_of(a) {
        var best = a[0], len = best.length;
        for (var i = 1; i < a.length; ++i) {
            if (a[i].length < len) {
                best = a[i];
                len = best.length;
            }
        }
        return best;
    }

    function make_num(num) {
        var str = num.toString(10).replace(/^0\./, ".").replace("e+", "e");
        var candidates = [ str ];
        if (Math.floor(num) === num) {
            if (num < 0) {
                candidates.push("-0x" + (-num).toString(16).toLowerCase());
            } else {
                candidates.push("0x" + num.toString(16).toLowerCase());
            }
        }
        var match, len, digits;
        if (match = /^\.0+/.exec(str)) {
            len = match[0].length;
            digits = str.slice(len);
            candidates.push(digits + "e-" + (digits.length + len - 1));
        } else if (match = /0+$/.exec(str)) {
            len = match[0].length;
            candidates.push(str.slice(0, -len) + "e" + len);
        } else if (match = /^(\d)\.(\d+)e(-?\d+)$/.exec(str)) {
            candidates.push(match[1] + match[2] + "e" + (match[3] - match[2].length));
        }
        return best_of(candidates);
    }

    function make_block(stmt, output) {
        if (!stmt || stmt instanceof AST_EmptyStatement)
            output.print("{}");
        else if (stmt instanceof AST_BlockStatement)
            stmt.print(output);
        else output.with_block(function() {
            output.indent();
            stmt.print(output);
            output.newline();
        });
    }

    /* -----[ source map generators ]----- */

    function DEFMAP(nodetype, generator) {
        nodetype.forEach(function(nodetype) {
            nodetype.DEFMETHOD("add_source_map", generator);
        });
    }

    DEFMAP([
        // We could easily add info for ALL nodes, but it seems to me that
        // would be quite wasteful, hence this noop in the base class.
        AST_Node,
        // since the label symbol will mark it
        AST_LabeledStatement,
        AST_Toplevel,
    ], noop);

    // XXX: I'm not exactly sure if we need it for all of these nodes,
    // or if we should add even more.
    DEFMAP([
        AST_Array,
        AST_BlockStatement,
        AST_Catch,
        AST_Class,
        AST_Constant,
        AST_Debugger,
        AST_Definitions,
        AST_Directive,
        AST_Finally,
        AST_Jump,
        AST_Lambda,
        AST_New,
        AST_Object,
        AST_StatementWithBody,
        AST_Symbol,
        AST_Switch,
        AST_SwitchBranch,
        AST_TemplateString,
        AST_TemplateSegment,
        AST_Try,
    ], function(output) {
        output.add_mapping(this.start);
    });

    DEFMAP([
        AST_ObjectGetter,
        AST_ObjectSetter,
        AST_PrivateGetter,
        AST_PrivateSetter,
    ], function(output) {
        output.add_mapping(this.key.end, this.key.name);
    });

    DEFMAP([ AST_ObjectProperty ], function(output) {
        output.add_mapping(this.start, this.key);
    });
})();

/***********************************************************************

  A JavaScript tokenizer / parser / beautifier / compressor.
  https://github.com/mishoo/UglifyJS2

  -------------------------------- (C) ---------------------------------

                           Author: Mihai Bazon
                         <mihai.bazon@gmail.com>
                       http://mihai.bazon.net/blog

  Distributed under the BSD license:

    Copyright 2012 (c) Mihai Bazon <mihai.bazon@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        * Redistributions of source code must retain the above
          copyright notice, this list of conditions and the following
          disclaimer.

        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials
          provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

 ***********************************************************************/

const MASK_EXPORT_DONT_MANGLE = 1 << 0;
const MASK_EXPORT_WANT_MANGLE = 1 << 1;

let function_defs = null;
let unmangleable_names = null;
/**
 * When defined, there is a function declaration somewhere that's inside of a block.
 * See https://tc39.es/ecma262/multipage/additional-ecmascript-features-for-web-browsers.html#sec-block-level-function-declarations-web-legacy-compatibility-semantics
*/
let scopes_with_block_defuns = null;

class SymbolDef {
    constructor(scope, orig, init) {
        this.name = orig.name;
        this.orig = [ orig ];
        this.init = init;
        this.eliminated = 0;
        this.assignments = 0;
        this.scope = scope;
        this.replaced = 0;
        this.global = false;
        this.export = 0;
        this.mangled_name = null;
        this.undeclared = false;
        this.id = SymbolDef.next_id++;
        this.chained = false;
        this.direct_access = false;
        this.escaped = 0;
        this.recursive_refs = 0;
        this.references = [];
        this.should_replace = undefined;
        this.single_use = false;
        this.fixed = false;
        Object.seal(this);
    }
    fixed_value() {
        if (!this.fixed || this.fixed instanceof AST_Node) return this.fixed;
        return this.fixed();
    }
    unmangleable(options) {
        if (!options) options = {};

        if (
            function_defs &&
            function_defs.has(this.id) &&
            keep_name(options.keep_fnames, this.orig[0].name)
        ) return true;

        return this.global && !options.toplevel
            || (this.export & MASK_EXPORT_DONT_MANGLE)
            || this.undeclared
            || !options.eval && this.scope.pinned()
            || (this.orig[0] instanceof AST_SymbolLambda
                  || this.orig[0] instanceof AST_SymbolDefun) && keep_name(options.keep_fnames, this.orig[0].name)
            || this.orig[0] instanceof AST_SymbolMethod
            || (this.orig[0] instanceof AST_SymbolClass
                  || this.orig[0] instanceof AST_SymbolDefClass) && keep_name(options.keep_classnames, this.orig[0].name);
    }
    mangle(options) {
        const cache = options.cache && options.cache.props;
        if (this.global && cache && cache.has(this.name)) {
            this.mangled_name = cache.get(this.name);
        } else if (!this.mangled_name && !this.unmangleable(options)) {
            var s = this.scope;
            var sym = this.orig[0];
            if (options.ie8 && sym instanceof AST_SymbolLambda)
                s = s.parent_scope;
            const redefinition = redefined_catch_def(this);
            this.mangled_name = redefinition
                ? redefinition.mangled_name || redefinition.name
                : s.next_mangled(options, this);
            if (this.global && cache) {
                cache.set(this.name, this.mangled_name);
            }
        }
    }
}

SymbolDef.next_id = 1;

function redefined_catch_def(def) {
    if (def.orig[0] instanceof AST_SymbolCatch
        && def.scope.is_block_scope()
    ) {
        return def.scope.get_defun_scope().variables.get(def.name);
    }
}

AST_Scope.DEFMETHOD("figure_out_scope", function(options, { parent_scope = null, toplevel = this } = {}) {
    options = defaults(options, {
        cache: null,
        ie8: false,
        safari10: false,
    });

    if (!(toplevel instanceof AST_Toplevel)) {
        throw new Error("Invalid toplevel scope");
    }

    // pass 1: setup scope chaining and handle definitions
    var scope = this.parent_scope = parent_scope;
    var labels = new Map();
    var defun = null;
    var in_destructuring = null;
    var for_scopes = [];
    var tw = new TreeWalker((node, descend) => {
        if (node.is_block_scope()) {
            const save_scope = scope;
            node.block_scope = scope = new AST_Scope(node);
            scope._block_scope = true;
            scope.init_scope_vars(save_scope);
            scope.uses_with = save_scope.uses_with;
            scope.uses_eval = save_scope.uses_eval;

            if (options.safari10) {
                if (node instanceof AST_For || node instanceof AST_ForIn || node instanceof AST_ForOf) {
                    for_scopes.push(scope);
                }
            }

            if (node instanceof AST_Switch) {
                // XXX: HACK! Ensure the switch expression gets the correct scope (the parent scope) and the body gets the contained scope
                // AST_Switch has a scope within the body, but it itself "is a block scope"
                // This means the switched expression has to belong to the outer scope
                // while the body inside belongs to the switch itself.
                // This is pretty nasty and warrants an AST change
                const the_block_scope = scope;
                scope = save_scope;
                node.expression.walk(tw);
                scope = the_block_scope;
                for (let i = 0; i < node.body.length; i++) {
                    node.body[i].walk(tw);
                }
            } else {
                descend();
            }
            scope = save_scope;
            return true;
        }
        if (node instanceof AST_Destructuring) {
            const save_destructuring = in_destructuring;
            in_destructuring = node;
            descend();
            in_destructuring = save_destructuring;
            return true;
        }
        if (node instanceof AST_Scope) {
            node.init_scope_vars(scope);
            var save_scope = scope;
            var save_defun = defun;
            var save_labels = labels;
            defun = scope = node;
            labels = new Map();
            descend();
            scope = save_scope;
            defun = save_defun;
            labels = save_labels;
            return true;        // don't descend again in TreeWalker
        }
        if (node instanceof AST_LabeledStatement) {
            var l = node.label;
            if (labels.has(l.name)) {
                throw new Error(string_template("Label {name} defined twice", l));
            }
            labels.set(l.name, l);
            descend();
            labels.delete(l.name);
            return true;        // no descend again
        }
        if (node instanceof AST_With) {
            for (var s = scope; s; s = s.parent_scope)
                s.uses_with = true;
            return;
        }
        if (node instanceof AST_Symbol) {
            node.scope = scope;
        }
        if (node instanceof AST_Label) {
            node.thedef = node;
            node.references = [];
        }
        if (node instanceof AST_SymbolLambda) {
            defun.def_function(node, node.name == "arguments" ? undefined : defun);
        } else if (node instanceof AST_SymbolDefun) {
            // Careful here, the scope where this should be defined is
            // the parent scope.  The reason is that we enter a new
            // scope when we encounter the AST_Defun node (which is
            // instanceof AST_Scope) but we get to the symbol a bit
            // later.
            const closest_scope = defun.parent_scope;

            // In strict mode, function definitions are block-scoped
            node.scope = tw.directives["use strict"]
                ? closest_scope
                : closest_scope.get_defun_scope();

            mark_export(node.scope.def_function(node, defun), 1);
        } else if (node instanceof AST_SymbolClass) {
            mark_export(defun.def_variable(node, defun), 1);
        } else if (node instanceof AST_SymbolImport) {
            scope.def_variable(node);
        } else if (node instanceof AST_SymbolDefClass) {
            // This deals with the name of the class being available
            // inside the class.
            mark_export((node.scope = defun.parent_scope).def_function(node, defun), 1);
        } else if (
            node instanceof AST_SymbolVar
            || node instanceof AST_SymbolLet
            || node instanceof AST_SymbolConst
            || node instanceof AST_SymbolCatch
        ) {
            var def;
            if (node instanceof AST_SymbolBlockDeclaration) {
                def = scope.def_variable(node, null);
            } else {
                def = defun.def_variable(node, node.TYPE == "SymbolVar" ? null : undefined);
            }
            if (!def.orig.every((sym) => {
                if (sym === node) return true;
                if (node instanceof AST_SymbolBlockDeclaration) {
                    return sym instanceof AST_SymbolLambda;
                }
                return !(sym instanceof AST_SymbolLet || sym instanceof AST_SymbolConst);
            })) {
                js_error(
                    `"${node.name}" is redeclared`,
                    node.start.file,
                    node.start.line,
                    node.start.col,
                    node.start.pos
                );
            }
            if (!(node instanceof AST_SymbolFunarg)) mark_export(def, 2);
            if (defun !== scope) {
                node.mark_enclosed();
                var def = scope.find_variable(node);
                if (node.thedef !== def) {
                    node.thedef = def;
                    node.reference();
                }
            }
        } else if (node instanceof AST_LabelRef) {
            var sym = labels.get(node.name);
            if (!sym) throw new Error(string_template("Undefined label {name} [{line},{col}]", {
                name: node.name,
                line: node.start.line,
                col: node.start.col
            }));
            node.thedef = sym;
        }
        if (!(scope instanceof AST_Toplevel) && (node instanceof AST_Export || node instanceof AST_Import)) {
            js_error(
                `"${node.TYPE}" statement may only appear at the top level`,
                node.start.file,
                node.start.line,
                node.start.col,
                node.start.pos
            );
        }
    });
    this.walk(tw);

    function mark_export(def, level) {
        if (in_destructuring) {
            var i = 0;
            do {
                level++;
            } while (tw.parent(i++) !== in_destructuring);
        }
        var node = tw.parent(level);
        if (def.export = node instanceof AST_Export ? MASK_EXPORT_DONT_MANGLE : 0) {
            var exported = node.exported_definition;
            if ((exported instanceof AST_Defun || exported instanceof AST_DefClass) && node.is_default) {
                def.export = MASK_EXPORT_WANT_MANGLE;
            }
        }
    }

    // pass 2: find back references and eval
    const is_toplevel = this instanceof AST_Toplevel;
    if (is_toplevel) {
        this.globals = new Map();
    }

    var tw = new TreeWalker(node => {
        if (node instanceof AST_LoopControl && node.label) {
            node.label.thedef.references.push(node);
            return true;
        }
        if (node instanceof AST_SymbolRef) {
            var name = node.name;
            if (name == "eval" && tw.parent() instanceof AST_Call) {
                for (var s = node.scope; s && !s.uses_eval; s = s.parent_scope) {
                    s.uses_eval = true;
                }
            }
            var sym;
            if (tw.parent() instanceof AST_NameMapping && tw.parent(1).module_name
                || !(sym = node.scope.find_variable(name))) {

                sym = toplevel.def_global(node);
                if (node instanceof AST_SymbolExport) sym.export = MASK_EXPORT_DONT_MANGLE;
            } else if (sym.scope instanceof AST_Lambda && name == "arguments") {
                sym.scope.get_defun_scope().uses_arguments = true;
            }
            node.thedef = sym;
            node.reference();
            if (node.scope.is_block_scope()
                && !(sym.orig[0] instanceof AST_SymbolBlockDeclaration)) {
                node.scope = node.scope.get_defun_scope();
            }
            return true;
        }
        // ensure mangling works if catch reuses a scope variable
        var def;
        if (node instanceof AST_SymbolCatch && (def = redefined_catch_def(node.definition()))) {
            var s = node.scope;
            while (s) {
                push_uniq(s.enclosed, def);
                if (s === def.scope) break;
                s = s.parent_scope;
            }
        }
    });
    this.walk(tw);

    // pass 3: work around IE8 and Safari catch scope bugs
    if (options.ie8 || options.safari10) {
        walk(this, node => {
            if (node instanceof AST_SymbolCatch) {
                var name = node.name;
                var refs = node.thedef.references;
                var scope = node.scope.get_defun_scope();
                var def = scope.find_variable(name)
                    || toplevel.globals.get(name)
                    || scope.def_variable(node);
                refs.forEach(function(ref) {
                    ref.thedef = def;
                    ref.reference();
                });
                node.thedef = def;
                node.reference();
                return true;
            }
        });
    }

    // pass 4: add symbol definitions to loop scopes
    // Safari/Webkit bug workaround - loop init let variable shadowing argument.
    // https://github.com/mishoo/UglifyJS2/issues/1753
    // https://bugs.webkit.org/show_bug.cgi?id=171041
    if (options.safari10) {
        for (const scope of for_scopes) {
            scope.parent_scope.variables.forEach(function(def) {
                push_uniq(scope.enclosed, def);
            });
        }
    }
});

AST_Toplevel.DEFMETHOD("def_global", function(node) {
    var globals = this.globals, name = node.name;
    if (globals.has(name)) {
        return globals.get(name);
    } else {
        var g = new SymbolDef(this, node);
        g.undeclared = true;
        g.global = true;
        globals.set(name, g);
        return g;
    }
});

AST_Scope.DEFMETHOD("init_scope_vars", function(parent_scope) {
    this.variables = new Map();         // map name to AST_SymbolVar (variables defined in this scope; includes functions)
    this.uses_with = false;             // will be set to true if this or some nested scope uses the `with` statement
    this.uses_eval = false;             // will be set to true if this or nested scope uses the global `eval`
    this.parent_scope = parent_scope;   // the parent scope
    this.enclosed = [];                 // a list of variables from this or outer scope(s) that are referenced from this or inner scopes
    this.cname = -1;                    // the current index for mangling functions/variables
});

AST_Scope.DEFMETHOD("conflicting_def", function (name) {
    return (
        this.enclosed.find(def => def.name === name)
        || this.variables.has(name)
        || (this.parent_scope && this.parent_scope.conflicting_def(name))
    );
});

AST_Scope.DEFMETHOD("conflicting_def_shallow", function (name) {
    return (
        this.enclosed.find(def => def.name === name)
        || this.variables.has(name)
    );
});

AST_Scope.DEFMETHOD("add_child_scope", function (scope) {
    // `scope` is going to be moved into `this` right now.
    // Update the required scopes' information

    if (scope.parent_scope === this) return;

    scope.parent_scope = this;

    // Propagate to this.uses_arguments from arrow functions
    if ((scope instanceof AST_Arrow) && !this.uses_arguments) {
        this.uses_arguments = walk(scope, node => {
            if (
                node instanceof AST_SymbolRef
                && node.scope instanceof AST_Lambda
                && node.name === "arguments"
            ) {
                return walk_abort;
            }

            if (node instanceof AST_Lambda && !(node instanceof AST_Arrow)) {
                return true;
            }
        });
    }

    this.uses_with = this.uses_with || scope.uses_with;
    this.uses_eval = this.uses_eval || scope.uses_eval;

    const scope_ancestry = (() => {
        const ancestry = [];
        let cur = this;
        do {
            ancestry.push(cur);
        } while ((cur = cur.parent_scope));
        ancestry.reverse();
        return ancestry;
    })();

    const new_scope_enclosed_set = new Set(scope.enclosed);
    const to_enclose = [];
    for (const scope_topdown of scope_ancestry) {
        to_enclose.forEach(e => push_uniq(scope_topdown.enclosed, e));
        for (const def of scope_topdown.variables.values()) {
            if (new_scope_enclosed_set.has(def)) {
                push_uniq(to_enclose, def);
                push_uniq(scope_topdown.enclosed, def);
            }
        }
    }
});

function find_scopes_visible_from(scopes) {
    const found_scopes = new Set();

    for (const scope of new Set(scopes)) {
        (function bubble_up(scope) {
            if (scope == null || found_scopes.has(scope)) return;

            found_scopes.add(scope);

            bubble_up(scope.parent_scope);
        })(scope);
    }

    return [...found_scopes];
}

// Creates a symbol during compression
AST_Scope.DEFMETHOD("create_symbol", function(SymClass, {
    source,
    tentative_name,
    scope,
    conflict_scopes = [scope],
    init = null
} = {}) {
    let symbol_name;

    conflict_scopes = find_scopes_visible_from(conflict_scopes);

    if (tentative_name) {
        // Implement hygiene (no new names are conflicting with existing names)
        tentative_name =
            symbol_name =
            tentative_name.replace(/(?:^[^a-z_$]|[^a-z0-9_$])/ig, "_");

        let i = 0;
        while (conflict_scopes.find(s => s.conflicting_def_shallow(symbol_name))) {
            symbol_name = tentative_name + "$" + i++;
        }
    }

    if (!symbol_name) {
        throw new Error("No symbol name could be generated in create_symbol()");
    }

    const symbol = make_node(SymClass, source, {
        name: symbol_name,
        scope
    });

    this.def_variable(symbol, init || null);

    symbol.mark_enclosed();

    return symbol;
});


AST_Node.DEFMETHOD("is_block_scope", return_false);
AST_Class.DEFMETHOD("is_block_scope", return_false);
AST_Lambda.DEFMETHOD("is_block_scope", return_false);
AST_Toplevel.DEFMETHOD("is_block_scope", return_false);
AST_SwitchBranch.DEFMETHOD("is_block_scope", return_false);
AST_Block.DEFMETHOD("is_block_scope", return_true);
AST_Scope.DEFMETHOD("is_block_scope", function () {
    return this._block_scope || false;
});
AST_IterationStatement.DEFMETHOD("is_block_scope", return_true);

AST_Lambda.DEFMETHOD("init_scope_vars", function() {
    AST_Scope.prototype.init_scope_vars.apply(this, arguments);
    this.uses_arguments = false;
    this.def_variable(new AST_SymbolFunarg({
        name: "arguments",
        start: this.start,
        end: this.end
    }));
});

AST_Arrow.DEFMETHOD("init_scope_vars", function() {
    AST_Scope.prototype.init_scope_vars.apply(this, arguments);
    this.uses_arguments = false;
});

AST_Symbol.DEFMETHOD("mark_enclosed", function() {
    var def = this.definition();
    var s = this.scope;
    while (s) {
        push_uniq(s.enclosed, def);
        if (s === def.scope) break;
        s = s.parent_scope;
    }
});

AST_Symbol.DEFMETHOD("reference", function() {
    this.definition().references.push(this);
    this.mark_enclosed();
});

AST_Scope.DEFMETHOD("find_variable", function(name) {
    if (name instanceof AST_Symbol) name = name.name;
    return this.variables.get(name)
        || (this.parent_scope && this.parent_scope.find_variable(name));
});

AST_Scope.DEFMETHOD("def_function", function(symbol, init) {
    var def = this.def_variable(symbol, init);
    if (!def.init || def.init instanceof AST_Defun) def.init = init;
    return def;
});

AST_Scope.DEFMETHOD("def_variable", function(symbol, init) {
    var def = this.variables.get(symbol.name);
    if (def) {
        def.orig.push(symbol);
        if (def.init && (def.scope !== symbol.scope || def.init instanceof AST_Function)) {
            def.init = init;
        }
    } else {
        def = new SymbolDef(this, symbol, init);
        this.variables.set(symbol.name, def);
        def.global = !this.parent_scope;
    }
    return symbol.thedef = def;
});

function next_mangled(scope, options) {
    let defun_scope;
    if (
        scopes_with_block_defuns
        && (defun_scope = scope.get_defun_scope())
        && scopes_with_block_defuns.has(defun_scope)
    ) {
        scope = defun_scope;
    }

    var ext = scope.enclosed;
    var nth_identifier = options.nth_identifier;
    out: while (true) {
        var m = nth_identifier.get(++scope.cname);
        if (ALL_RESERVED_WORDS.has(m)) continue; // skip over "do"

        // https://github.com/mishoo/UglifyJS2/issues/242 -- do not
        // shadow a name reserved from mangling.
        if (options.reserved.has(m)) continue;

        // Functions with short names might collide with base54 output
        // and therefore cause collisions when keep_fnames is true.
        if (unmangleable_names && unmangleable_names.has(m)) continue out;

        // we must ensure that the mangled name does not shadow a name
        // from some parent scope that is referenced in this or in
        // inner scopes.
        for (let i = ext.length; --i >= 0;) {
            const def = ext[i];
            const name = def.mangled_name || (def.unmangleable(options) && def.name);
            if (m == name) continue out;
        }
        return m;
    }
}

AST_Scope.DEFMETHOD("next_mangled", function(options) {
    return next_mangled(this, options);
});

AST_Toplevel.DEFMETHOD("next_mangled", function(options) {
    let name;
    const mangled_names = this.mangled_names;
    do {
        name = next_mangled(this, options);
    } while (mangled_names.has(name));
    return name;
});

AST_Function.DEFMETHOD("next_mangled", function(options, def) {
    // #179, #326
    // in Safari strict mode, something like (function x(x){...}) is a syntax error;
    // a function expression's argument cannot shadow the function expression's name

    var tricky_def = def.orig[0] instanceof AST_SymbolFunarg && this.name && this.name.definition();

    // the function's mangled_name is null when keep_fnames is true
    var tricky_name = tricky_def ? tricky_def.mangled_name || tricky_def.name : null;

    while (true) {
        var name = next_mangled(this, options);
        if (!tricky_name || tricky_name != name)
            return name;
    }
});

AST_Symbol.DEFMETHOD("unmangleable", function(options) {
    var def = this.definition();
    return !def || def.unmangleable(options);
});

// labels are always mangleable
AST_Label.DEFMETHOD("unmangleable", return_false);

AST_Symbol.DEFMETHOD("unreferenced", function() {
    return !this.definition().references.length && !this.scope.pinned();
});

AST_Symbol.DEFMETHOD("definition", function() {
    return this.thedef;
});

AST_Symbol.DEFMETHOD("global", function() {
    return this.thedef.global;
});

/**
 * Format the mangler options (if any) into their appropriate types
 */
function format_mangler_options(options) {
    options = defaults(options, {
        eval        : false,
        nth_identifier : base54,
        ie8         : false,
        keep_classnames: false,
        keep_fnames : false,
        module      : false,
        reserved    : [],
        toplevel    : false,
    });
    if (options.module) options.toplevel = true;
    if (!Array.isArray(options.reserved)
        && !(options.reserved instanceof Set)
    ) {
        options.reserved = [];
    }
    options.reserved = new Set(options.reserved);
    // Never mangle arguments
    options.reserved.add("arguments");
    return options;
}

AST_Toplevel.DEFMETHOD("mangle_names", function(options) {
    options = format_mangler_options(options);
    var nth_identifier = options.nth_identifier;

    // We only need to mangle declaration nodes.  Special logic wired
    // into the code generator will display the mangled name if it's
    // present (and for AST_SymbolRef-s it'll use the mangled name of
    // the AST_SymbolDeclaration that it points to).
    var lname = -1;
    var to_mangle = [];

    if (options.keep_fnames) {
        function_defs = new Set();
    }

    const mangled_names = this.mangled_names = new Set();
    unmangleable_names = new Set();

    if (options.cache) {
        this.globals.forEach(collect);
        if (options.cache.props) {
            options.cache.props.forEach(function(mangled_name) {
                mangled_names.add(mangled_name);
            });
        }
    }

    var tw = new TreeWalker(function(node, descend) {
        if (node instanceof AST_LabeledStatement) {
            // lname is incremented when we get to the AST_Label
            var save_nesting = lname;
            descend();
            lname = save_nesting;
            return true;        // don't descend again in TreeWalker
        }
        if (
            node instanceof AST_Defun
            && !(tw.parent() instanceof AST_Scope)
        ) {
            scopes_with_block_defuns = scopes_with_block_defuns || new Set();
            scopes_with_block_defuns.add(node.parent_scope.get_defun_scope());
        }
        if (node instanceof AST_Scope) {
            node.variables.forEach(collect);
            return;
        }
        if (node.is_block_scope()) {
            node.block_scope.variables.forEach(collect);
            return;
        }
        if (
            function_defs
            && node instanceof AST_VarDef
            && node.value instanceof AST_Lambda
            && !node.value.name
            && keep_name(options.keep_fnames, node.name.name)
        ) {
            function_defs.add(node.name.definition().id);
            return;
        }
        if (node instanceof AST_Label) {
            let name;
            do {
                name = nth_identifier.get(++lname);
            } while (ALL_RESERVED_WORDS.has(name));
            node.mangled_name = name;
            return true;
        }
        if (!(options.ie8 || options.safari10) && node instanceof AST_SymbolCatch) {
            to_mangle.push(node.definition());
            return;
        }
    });

    this.walk(tw);

    if (options.keep_fnames || options.keep_classnames) {
        // Collect a set of short names which are unmangleable,
        // for use in avoiding collisions in next_mangled.
        to_mangle.forEach(def => {
            if (def.name.length < 6 && def.unmangleable(options)) {
                unmangleable_names.add(def.name);
            }
        });
    }

    to_mangle.forEach(def => { def.mangle(options); });

    function_defs = null;
    unmangleable_names = null;
    scopes_with_block_defuns = null;

    function collect(symbol) {
        if (symbol.export & MASK_EXPORT_DONT_MANGLE) {
            unmangleable_names.add(symbol.name);
        } else if (!options.reserved.has(symbol.name)) {
            to_mangle.push(symbol);
        }
    }
});

AST_Toplevel.DEFMETHOD("find_colliding_names", function(options) {
    const cache = options.cache && options.cache.props;
    const avoid = new Set();
    options.reserved.forEach(to_avoid);
    this.globals.forEach(add_def);
    this.walk(new TreeWalker(function(node) {
        if (node instanceof AST_Scope) node.variables.forEach(add_def);
        if (node instanceof AST_SymbolCatch) add_def(node.definition());
    }));
    return avoid;

    function to_avoid(name) {
        avoid.add(name);
    }

    function add_def(def) {
        var name = def.name;
        if (def.global && cache && cache.has(name)) name = cache.get(name);
        else if (!def.unmangleable(options)) return;
        to_avoid(name);
    }
});

AST_Toplevel.DEFMETHOD("expand_names", function(options) {
    options = format_mangler_options(options);
    var nth_identifier = options.nth_identifier;
    if (nth_identifier.reset && nth_identifier.sort) {
        nth_identifier.reset();
        nth_identifier.sort();
    }
    var avoid = this.find_colliding_names(options);
    var cname = 0;
    this.globals.forEach(rename);
    this.walk(new TreeWalker(function(node) {
        if (node instanceof AST_Scope) node.variables.forEach(rename);
        if (node instanceof AST_SymbolCatch) rename(node.definition());
    }));

    function next_name() {
        var name;
        do {
            name = nth_identifier.get(cname++);
        } while (avoid.has(name) || ALL_RESERVED_WORDS.has(name));
        return name;
    }

    function rename(def) {
        if (def.global && options.cache) return;
        if (def.unmangleable(options)) return;
        if (options.reserved.has(def.name)) return;
        const redefinition = redefined_catch_def(def);
        const name = def.name = redefinition ? redefinition.name : next_name();
        def.orig.forEach(function(sym) {
            sym.name = name;
        });
        def.references.forEach(function(sym) {
            sym.name = name;
        });
    }
});

AST_Node.DEFMETHOD("tail_node", return_this);
AST_Sequence.DEFMETHOD("tail_node", function() {
    return this.expressions[this.expressions.length - 1];
});

AST_Toplevel.DEFMETHOD("compute_char_frequency", function(options) {
    options = format_mangler_options(options);
    var nth_identifier = options.nth_identifier;
    if (!nth_identifier.reset || !nth_identifier.consider || !nth_identifier.sort) {
        // If the identifier mangler is invariant, skip computing character frequency.
        return;
    }
    nth_identifier.reset();

    try {
        AST_Node.prototype.print = function(stream, force_parens) {
            this._print(stream, force_parens);
            if (this instanceof AST_Symbol && !this.unmangleable(options)) {
                nth_identifier.consider(this.name, -1);
            } else if (options.properties) {
                if (this instanceof AST_DotHash) {
                    nth_identifier.consider("#" + this.property, -1);
                } else if (this instanceof AST_Dot) {
                    nth_identifier.consider(this.property, -1);
                } else if (this instanceof AST_Sub) {
                    skip_string(this.property);
                }
            }
        };
        nth_identifier.consider(this.print_to_string(), 1);
    } finally {
        AST_Node.prototype.print = AST_Node.prototype._print;
    }
    nth_identifier.sort();

    function skip_string(node) {
        if (node instanceof AST_String) {
            nth_identifier.consider(node.value, -1);
        } else if (node instanceof AST_Conditional) {
            skip_string(node.consequent);
            skip_string(node.alternative);
        } else if (node instanceof AST_Sequence) {
            skip_string(node.tail_node());
        }
    }
});

const base54 = (() => {
    const leading = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$_".split("");
    const digits = "0123456789".split("");
    let chars;
    let frequency;
    function reset() {
        frequency = new Map();
        leading.forEach(function(ch) {
            frequency.set(ch, 0);
        });
        digits.forEach(function(ch) {
            frequency.set(ch, 0);
        });
    }
    function consider(str, delta) {
        for (var i = str.length; --i >= 0;) {
            frequency.set(str[i], frequency.get(str[i]) + delta);
        }
    }
    function compare(a, b) {
        return frequency.get(b) - frequency.get(a);
    }
    function sort() {
        chars = mergeSort(leading, compare).concat(mergeSort(digits, compare));
    }
    // Ensure this is in a usable initial state.
    reset();
    sort();
    function base54(num) {
        var ret = "", base = 54;
        num++;
        do {
            num--;
            ret += chars[num % base];
            num = Math.floor(num / base);
            base = 64;
        } while (num > 0);
        return ret;
    }

    return {
        get: base54,
        consider,
        reset,
        sort
    };
})();

exports.AST_Node = AST_Node;
exports.AST_Token = AST_Token;

})));
