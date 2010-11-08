/*
GLGE WebGL Graphics Engine
Copyright (c) 2010, Paul Brunt
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of GLGE nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL PAUL BRUNT BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @fileOverview
 * @name GLGE_math.js
 */

 if(!window["GLGE"]){
	/**
	* @namespace Holds the functionality of the library
	*/
	window["GLGE"]={};
}

(function(GLGE){

GLGE.Vec=function(array) {
    return array.slice(0);
}

/**
* The Vec3 Class creates a vector 
* @param {Array} array An array of 3 floats
*/
GLGE.Vec3=function(x,y,z){
    return [x,y,z];
}

/**
* The Vec4 Class creates a vector 
* @param {Array} array An array of 4 floats
*/
GLGE.Vec4=function(x,y,z,w){
    return [x,y,z,w];
}

/**
* Gets the nth element (1 indexed) from the array
* @param {Array} v A vector with 4 elements
* @param {number} i The index from one 
*/
GLGE.get1basedVec4=function(v,i){
	return v[i-1];
};
/**
* Gets the nth element (1 indexed) from the array
* @param {Array} v A vector with 3 elements
* @param {number} i The index from one 
*/
GLGE.get1basedVec3=function(v,i){
	return v[i-1];
};

/**
* Gets the nth element (1 indexed) from the array
* @param {Array} v A vector with 4 elements
* @param {number} i The index from one 
*/
GLGE.getVec4=function(v,i){
	return v[i];
};
/**
* Gets the nth element (1 indexed) from the array
* @param {Array} v A vector with 3 elements
* @param {number} i The index from one 
*/
GLGE.getVec3=function(v,i){
	return v[i];
};



/**
* Adds a GLGE.Vec4 to this Vec4
* @param {Array} a The first value to add
* * @param {Array} b The second value to add
*/
GLGE.addVec4=function(a,b) {
    return [a[0]+b[0],a[1]+b[1],a[2]+b[2],a[3]+b[3]];
}
/**
* Adds a GLGE.Vec3 to this GLGE.Vec3
* @param {Array} a The first value to add
* @param {Array} b The second value to add
*/
GLGE.addVec3=function(a,b) {
    return [a[0]+b[0],a[1]+b[1],a[2]+b[2]];
}


/**
* Adds a GLGE.Vec4 to this Vec4
* @param {Array} a The first value
* * @param {Array} b The second value to subtract from the first
*/
GLGE.subVec4=function(a,b) {
    return [a[0]-b[0],a[1]-b[1],a[2]-b[2],a[3]-b[3]];
}
/**
* Adds a GLGE.Vec3 to this GLGE.Vec3
* @param {Array} a The first value
* @param {Array} b The second value to subtract from the first
*/
GLGE.subVec3=function(a,b) {
    return [a[0]-b[0],a[1]-b[1],a[2]-b[2]];
}


/**
* Gets the dot product between this and the input vector
* @param {Array} a the first value to dot
* @param {Array} b the second value to dot
*/
GLGE.dotVec3=function(a,b) {
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}


/**
* Gets the dot product between this and the input vector
* @param {Array} a the first value to dot
* @param {Array} b the second value to dot
*/
GLGE.dotVec4=function(a,b) {
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
}

/**
* Gets the dot product between this and the input vector
* @param {Array} a the vector to scale
* @param {Number} b the scalar
*/
GLGE.scaleVec4=function(a,b) {
    return [a[0]*b,a[1]*b,a[2]*b,a[3]*b];
}

/**
* Gets the dot product between this and the input vector
* @param {Array} a the vector to scale
* @param {Number} b the scalar
*/
GLGE.scaleVec3=function(a,b) {
    return [a[0]*b,a[1]*b,a[2]*b];
}


/**
* Gets the cross product between this and the input vector
* @param {Array} a the first value to dot
* @param {Array} b the second value to dot
*/
GLGE.crossVec3=function(a,b) {
  return [a[1]*b[2]-a[2]*b[1],
          a[2]*b[0]-a[0]*b[2],
          a[0]*b[1]-a[1]*b[0]];
}

/**
* Returns a unitized version of the input vector3
* @param {Array} a the vector3 to be unitized
*/
GLGE.toUnitVec3=function(a) {
    var sq=a[0]*a[0]+a[1]*a[1]+a[2]*a[2];
    var f=1.0;
    if (sq>0) {
        f=Math.pow(sq,0.5);
    }
    return [a[0]/f,a[1]/f,a[2]/f];
};

/**
* Returns a unitized version of the input vector4
* @param {Array} a the vector4 to be unitized
*/
GLGE.toUnitVec4=function(a) {
    var sq=a[0]*a[0]+a[1]*a[1]+a[2]*a[2]+a[3]*a[3];
    var f=1.0;
    if (sq>0) {
        f=Math.pow(sq,0.5);
    }
    return [a[0]/f,a[1]/f,a[2]/f,a[3]/f];
};


/**
* Returns the length of a vector3
* @param {Array} a the vector to be measured
*/
GLGE.lengthVec3=function(a) {
    return Math.pow(a[0]*a[0]+a[1]*a[1]+a[2]*a[2],0.5);
};

/**
* Returns the distance between 2 vector3s
* @param {Array} a the first vector
* @param {Array} b the second vector
*/
GLGE.distanceVec3=function(a,b){
    return GLGE.lengthVec3(GLGE.subVec3(a,b));
};

/**
* Returns the length of a vector3
* @param {Array} a the vector to be measured
*/
GLGE.lengthVec4=function(a,b) {
    return Math.pow(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]+a[3]*a[3],0.5);
};

/**
* Returns the distance between 2 vector4s
* @param {Array} a the first vector
* @param {Array} b the second vector
*/
GLGE.distanceVec4=function(a,b){
    return GLGE.lengthVec4(GLGE.subVec4(a,b));
};


/**
* Returns the angle between 2 vector3s in radians
* @param {Array} a the first vector
* @param {Array} b the second vector
*/
GLGE.angleVec3=function(a,b){
    a=GLGE.toUnitVec3(a);
    b=GLGE.toUnitVec3(b);
    d=GLGE.dotVec3(a,b);
    if (d<-1)
        d=-1;
    if (d>1)
        d=1;
    return Math.acos(d);
};

/**
* Returns the angle between 2 vector4s in radians
* @param {Array} a the first vector
* @param {Array} b the second vector
*/
GLGE.angleVec4=function(a,b){
    a=GLGE.toUnitVec4(a);
    b=GLGE.toUnitVec4(b);
    d=GLGE.dotVec4(a,b);
    if (d<-1)
        d=-1;
    if (d>1)
        d=1;
    return Math.acos(d);
};

GLGE_math_use_webgl_float=false;

/**
* The Mat class creates a matrix from an array
* @param {Array} array An array of 9 or 16 floats
*/
GLGE.Mat3=GLGE_math_use_webgl_float?function(array) {
    if (array.length==9) {
        return new Float32Array(array);
    }else if (array.length==16) {
        return new Float32Array([array[0],array[1],array[2],array[4],array[5],array[6],array[8],array[9],array[10]]);        
    }else {
		throw "invalid matrix length";
    }
}:function(array) {
    var retval;
    if (array.length==9) {
        retval=array.slice(0);
    }else if (array.length==16) {
        retval=[array[0],array[1],array[2],array[4],array[5],array[6],array[8],array[9],array[10]];
    }else {
		throw "invalid matrix length";
    }    
    retval.get=function(i){return this[i];};
    return retval;
};
GLGE.Mat=GLGE_math_use_webgl_float?function(array) {
    return new Float32Array(array);
}:function(array){
    var retval=array.slice(0);
    retval.get=function(i){return this[i];};
    return retval;
};
GLGE.Mat4=function(array) {
    var retval;
    if (array.length==9) {
        retval=[array[0],array[1],array[2],0,array[3],array[4],array[5],0,array[6],array[7],array[8],0,0,0,0,1];
    }else if (array.length==16) {
        retval=array.slice(0);
    }else {
        throw "invalid matrix length";
    }
    retval.get=function(i){return this[i];};
    return retval;
};
/**
* Finds the determinate of the matrix
* @returns {number} the determinate
*/
GLGE.determinantMat4=function(m) {
    return m[12] * m[9] * m[6] * m[3] - m[8] * m[13] * m[6] * m[3] - m[12] * m[5] * m[10] * m[3] + m[4] * m[13] * m[10] * m[3] + m[8] * m[5] * m[14] * m[3] - m[4] * m[9] * m[14] * m[3] - m[12] * m[9] * m[2] * m[7] + m[8] * m[13] * m[2] * m[7] + m[12] * m[1] * m[10] * m[7] - m[0] * m[13] * m[10] * m[7] - m[8] * m[1] * m[14] * m[7] + m[0] * m[9] * m[14] * m[7] + m[12] * m[5] * m[2] * m[11] - m[4] * m[13] * m[2] * m[11] - m[12] * m[1] * m[6] * m[11] + m[0] * m[13] * m[6] * m[11] + m[4] * m[1] * m[14] * m[11] - m[0] * m[5] * m[14] * m[11] - m[8] * m[5] * m[2] * m[15] + m[4] * m[9] * m[2] * m[15] + m[8] * m[1] * m[6] * m[15] - m[0] * m[9] * m[6] * m[15] - m[4] * m[1] * m[10] * m[15] + m[0] * m[5] * m[10] * m[15];
};

/**
* Finds the inverse of the matrix
* @returns {GLGE.Mat} the inverse
*/
GLGE.inverseMat4=function(mat){
	// Cache the matrix values (makes for huge speed increases!)
	var a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3];
	var a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7];
	var a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11];
	var a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15];
	
	var d = a30*a21*a12*a03 - a20*a31*a12*a03 - a30*a11*a22*a03 + a10*a31*a22*a03 +
			a20*a11*a32*a03 - a10*a21*a32*a03 - a30*a21*a02*a13 + a20*a31*a02*a13 +
			a30*a01*a22*a13 - a00*a31*a22*a13 - a20*a01*a32*a13 + a00*a21*a32*a13 +
			a30*a11*a02*a23 - a10*a31*a02*a23 - a30*a01*a12*a23 + a00*a31*a12*a23 +
			a10*a01*a32*a23 - a00*a11*a32*a23 - a20*a11*a02*a33 + a10*a21*a02*a33 +
			a20*a01*a12*a33 - a00*a21*a12*a33 - a10*a01*a22*a33 + a00*a11*a22*a33;
	
	return [ (a21*a32*a13 - a31*a22*a13 + a31*a12*a23 - a11*a32*a23 - a21*a12*a33 + a11*a22*a33)/d,
	(a31*a22*a03 - a21*a32*a03 - a31*a02*a23 + a01*a32*a23 + a21*a02*a33 - a01*a22*a33)/d,
	(a11*a32*a03 - a31*a12*a03 + a31*a02*a13 - a01*a32*a13 - a11*a02*a33 + a01*a12*a33)/d,
	(a21*a12*a03 - a11*a22*a03 - a21*a02*a13 + a01*a22*a13 + a11*a02*a23 - a01*a12*a23)/d,
	(a30*a22*a13 - a20*a32*a13 - a30*a12*a23 + a10*a32*a23 + a20*a12*a33 - a10*a22*a33)/d,
	(a20*a32*a03 - a30*a22*a03 + a30*a02*a23 - a00*a32*a23 - a20*a02*a33 + a00*a22*a33)/d,
	(a30*a12*a03 - a10*a32*a03 - a30*a02*a13 + a00*a32*a13 + a10*a02*a33 - a00*a12*a33)/d,
	(a10*a22*a03 - a20*a12*a03 + a20*a02*a13 - a00*a22*a13 - a10*a02*a23 + a00*a12*a23)/d,
	(a20*a31*a13 - a30*a21*a13 + a30*a11*a23 - a10*a31*a23 - a20*a11*a33 + a10*a21*a33)/d,
	(a30*a21*a03 - a20*a31*a03 - a30*a01*a23 + a00*a31*a23 + a20*a01*a33 - a00*a21*a33)/d,
	(a10*a31*a03 - a30*a11*a03 + a30*a01*a13 - a00*a31*a13 - a10*a01*a33 + a00*a11*a33)/d,
	(a20*a11*a03 - a10*a21*a03 - a20*a01*a13 + a00*a21*a13 + a10*a01*a23 - a00*a11*a23)/d,
	(a30*a21*a12 - a20*a31*a12 - a30*a11*a22 + a10*a31*a22 + a20*a11*a32 - a10*a21*a32)/d,
	(a20*a31*a02 - a30*a21*a02 + a30*a01*a22 - a00*a31*a22 - a20*a01*a32 + a00*a21*a32)/d,
	(a30*a11*a02 - a10*a31*a02 - a30*a01*a12 + a00*a31*a12 + a10*a01*a32 - a00*a11*a32)/d,
	(a10*a21*a02 - a20*a11*a02 + a20*a01*a12 - a00*a21*a12 - a10*a01*a22 + a00*a11*a22)/d]
};

/**
* multiplies two mat4's
* @returns {GLGE.Mat} the matrix multiplication of the matrices
*/
GLGE.mulMat4Vec4=function(mat1,vec2){
	return GLGE.Vec4(mat1[0]*vec2[0]+mat1[1]*vec2[1]+mat1[2]*vec2[2]+mat1[3]*vec2[3],
			          mat1[4]*vec2[0]+mat1[5]*vec2[1]+mat1[6]*vec2[2]+mat1[7]*vec2[3],
			          mat1[8]*vec2[0]+mat1[9]*vec2[1]+mat1[10]*vec2[2]+mat1[11]*vec2[3],
			          mat1[12]*vec2[0]+mat1[13]*vec2[1]+mat1[14]*vec2[2]+mat1[15]*vec2[3]);
};
     
/**
* multiplies a Mat4 by a scalar value
* @returns {GLGE.Mat} the matrix multiplication of the matrices
*/
GLGE.scaleMat4=function(m,value) {
    return GLGE.Mat([m[0]*value,m[1]*value,m[2]*value,m[3]*value,
                                m[4]*value,m[5]*value,m[6]*value,m[7]*value,
                                m[8]*value,m[9]*value,m[10]*value,m[11]*value,
                                m[12]*value,m[13]*value,m[14]*value,m[15]*value]);
};
/**
* multiplies a Mat4 by a scalar value in place without allocation
* @returns {GLGE.Mat} the input matrix, modified
*/
GLGE.scaleInPlaceMat4=function(m,value) {
    m.set(0,m[0]*value);
    m.set(1,m[1]*value);
    m.set(2,m[2]*value);
    m.set(3,m[3]*value);
    m.set(4,m[4]*value);
    m.set(5,m[5]*value);
    m.set(6,m[6]*value);
    m.set(7,m[7]*value);
    m.set(8,m[8]*value);
    m.set(9,m[9]*value);
    m.set(10,m[10]*value);
    m.set(11,m[11]*value);
    m.set(12,m[12]*value);
    m.set(13,m[13]*value);
    m.set(14,m[14]*value);
    m.set(15,m[15]*value);
    return m;
};

/**
* adds a Mat4 to another Mat4 in place without allocation
* @returns {GLGE.Mat} the first input matrix, modified to be added
*/
GLGE.addInPlaceMat4=function(m,value) {
    m.set(0,m[0]+value[0]);
    m.set(1,m[1]+value[1]);
    m.set(2,m[2]+value[2]);
    m.set(3,m[3]+value[3]);
    m.set(4,m[4]+value[4]);
    m.set(5,m[5]+value[5]);
    m.set(6,m[6]+value[6]);
    m.set(7,m[7]+value[7]);
    m.set(8,m[8]+value[8]);
    m.set(9,m[9]+value[9]);
    m.set(10,m[10]+value[10]);
    m.set(11,m[11]+value[11]);
    m.set(12,m[12]+value[12]);
    m.set(13,m[13]+value[13]);
    m.set(14,m[14]+value[14]);
    m.set(15,m[15]+value[15]);
    return m;
};



/**
* adds two Mat4 together
* @returns {GLGE.Mat} a new, added Mat4
*/
GLGE.addMat4=function(m,value) {
return GLGE.Mat([m[0]+value[0],
                 m[1]+value[1],
                 m[2]+value[2],
                 m[3]+value[3],
                 m[4]+value[4],
                 m[5]+value[5],
                 m[6]+value[6],
                 m[7]+value[7],
                 m[8]+value[8],
                 m[9]+value[9],
                 m[10]+value[10],
                 m[11]+value[11],
                 m[12]+value[12],
                 m[13]+value[13],
                 m[14]+value[14],
                 m[15]+value[15]]);
    return m;
};



/**
* subs a Mat4 from another Mat4 in place without allocation
* @returns {GLGE.Mat} the first input matrix, modified to have the second subtacted
*/
GLGE.subInPlaceMat4=function(m,value) {
    m.set(0,m[0]-value[0]);
    m.set(1,m[1]-value[1]);
    m.set(2,m[2]-value[2]);
    m.set(3,m[3]-value[3]);
    m.set(4,m[4]-value[4]);
    m.set(5,m[5]-value[5]);
    m.set(6,m[6]-value[6]);
    m.set(7,m[7]-value[7]);
    m.set(8,m[8]-value[8]);
    m.set(9,m[9]-value[9]);
    m.set(10,m[10]-value[10]);
    m.set(11,m[11]-value[11]);
    m.set(12,m[12]-value[12]);
    m.set(13,m[13]-value[13]);
    m.set(14,m[14]-value[14]);
    m.set(15,m[15]-value[15]);
    return m;
};



/**
* subtracts the second matrix from the first
* @returns {GLGE.Mat} a new, subed Mat4
*/
GLGE.subMat4=function(m,value) {
return GLGE.Mat([m[0]-value[0],
                 m[1]-value[1],
                 m[2]-value[2],
                 m[3]-value[3],
                 m[4]-value[4],
                 m[5]-value[5],
                 m[6]-value[6],
                 m[7]-value[7],
                 m[8]-value[8],
                 m[9]-value[9],
                 m[10]-value[10],
                 m[11]-value[11],
                 m[12]-value[12],
                 m[13]-value[13],
                 m[14]-value[14],
                 m[15]-value[15]]);
    return m;
};


/**
* Finds the matrix multiplication with another GLGE.Mat or GLGE.vec or an Array of length 3-4
* @param {object} value An GLGE.Mat, GLGE.vec or Array
* @returns {GLGE.Mat|GLGE.Vec}
*/
GLGE.mulMat4=function(mat2,mat1){

	var a00 = mat1[0], a01 = mat1[1], a02 = mat1[2], a03 = mat1[3];
	var a10 = mat1[4], a11 = mat1[5], a12 = mat1[6], a13 = mat1[7];
	var a20 = mat1[8], a21 = mat1[9], a22 = mat1[10], a23 = mat1[11];
	var a30 = mat1[12], a31 = mat1[13], a32 = mat1[14], a33 = mat1[15];
	
	var b00 = mat2[0], b01 = mat2[1], b02 = mat2[2], b03 = mat2[3];
	var b10 = mat2[4], b11 = mat2[5], b12 = mat2[6], b13 = mat2[7];
	var b20 = mat2[8], b21 = mat2[9], b22 = mat2[10], b23 = mat2[11];
	var b30 = mat2[12], b31 = mat2[13], b32 = mat2[14], b33 = mat2[15];
	return [b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30,
		b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31,
		b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32,
		b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33,
		
		b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30,
		b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31,
		b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32,
		b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33,
		
		b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30,
		b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31,
		b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32,
		b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33,
		
		b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30,
		b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31,
		b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32,
		b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33];
};

GLGE.transposeInPlaceMat4=function(m) {
    var v=m[1];
    m.set(1,m[4]);
    m.set(4,v);


    v=m[8];
    m.set(8,m[2]);
    m.set(2,v);
    

    v=m[3];
    m.set(3,m[12]);
    m.set(12,v);

    v=m[9];
    m.set(9,m[6]);
    m.set(6,v);

    v=m[13];
    m.set(13,m[7]);
    m.set(7,v);

    v=m[14];
    m.set(14,m[11]);
    m.set(11,v);
    
};

/**
* Builds the transpose of the matrix
* @returns {GLGE.Mat} the transposed matrix
*/
GLGE.transposeMat4=function(m) {
    return GLGE.Mat4([m[0],m[4],m[8],m[12],
		              m[1],m[5],m[9],m[13],
		              m[2],m[6],m[10],m[14],
		              m[3],m[7],m[11],m[15]]);
};

/**
* copys a js array into a webglarray
* @param {array} mat the source array
* @param {webglarray} glarray the destination array
*/
GLGE.mat4gl=function(mat,glarray){
	glarray[0]=mat[0];
	glarray[1]=mat[1];
	glarray[2]=mat[2];
	glarray[3]=mat[3];
	glarray[4]=mat[4];
	glarray[5]=mat[5];
	glarray[6]=mat[6];
	glarray[7]=mat[7];
	glarray[8]=mat[8];
	glarray[9]=mat[9];
	glarray[10]=mat[10];
	glarray[11]=mat[11];
	glarray[12]=mat[12];
	glarray[13]=mat[13];
	glarray[14]=mat[14];
	glarray[15]=mat[15];
};

/**
* Sets the value at the specified index
* @param {number} i the first index 1 offset
* @param {number} j the second index 1 offset
* @param {number} value the value to set
*/
GLGE.set1basedMat4=function(m,i,j,value){
	m[(i-1)*4+(j-1)]=value;
    if(m.glData!==undefined){
        delete m.glData;
    }
};

/**
* Sets the value at the specified index
* @param {number} i the first index from zero
* @param {number} j the second index from zero
* @param {number} value the value to set
*/
GLGE.setMat4=function(m,i,j,value){
	m[i*4+j]=value;
    if(m.glData!==undefined){
        delete m.glData;
    }
};

/**
* Gets the value at the specified index
* @param {number} i the first index from one
* @param {number} j the second index from one
* @returns {number} the value at the given index
*/
GLGE.get1basedMat4=function(m,i,j){
	return m.get((i-1)*4+(j-1));
};

/**
* Gets the value at the specified index
* @param {number} i the first index from zero
* @param {number} j the second index from zero
* @returns {number} the value at the given index
*/
GLGE.getMat4=function(m,i,j){
	return m[i*4+j];
};
/**
* gets the a webgl float array for this Matrix, once generated it will cache it so it doesn't need to recreate everytime
* @returns {Float32Array} the webgl array for this Matrix
* @private
*/
GLGE.glDataMat4=function(m) {
    m.glArray=new Float32Array(m);
    return m.glArray;
};
/**
 * Creates an identity matrix
 * @returns {GLGE.Mat} the identity matrix
 */
GLGE.identMatrix=function(){
	return GLGE.Mat([1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]);
};
/**
 * Creates a translation matrix
 * @returns {Array} value an array GLGE.Vec or 3 paramters
 * @returns {GLGE.Mat} the translation matrix
 */
GLGE.translateMatrix=function(value){
	var x;
	var y;
	var z;
	if(arguments.length==3){
		x=arguments[0];
		y=arguments[1];
		z=arguments[2];
	}
	else if(value.data){
		x=value.data[0];
		y=value.data[1];
		z=value.data[2];
	}
	else if(value instanceof Array){
		x=value[0];
		y=value[1];
		z=value[2];
	}
	return GLGE.Mat([
		1,0,0,x,
		0,1,0,y,
		0,0,1,z,
		0,0,0,1
		]);
};
/**
 * Creates a scale matrix
 * @returns {Array} value an array GLGE.Vec or 3 paramters
 * @returns {GLGE.Mat} the scale matrix
 */
GLGE.scaleMatrix=function(value){
    var x;
    var y;
    var z;
	if(arguments.length==3){
		x=arguments[0];
		y=arguments[1];
		z=arguments[2];
	}
	else if(value.data){
		x=value.data[0];
		y=value.data[1];
		z=value.data[2];
	}
	else if(value instanceof Array){
		x=value[0];
		y=value[1];
		z=value[2];
	}
	return GLGE.Mat([
		x,0,0,0,
		0,y,0,0,
		0,0,z,0,
		0,0,0,1
		]);
}
/**
* @constant 
* @description Enum for XYZ rotation order
*/
GLGE.ROT_XYZ=1;
/**
* @constant 
* @description Enum for XZY rotation order
*/
GLGE.ROT_XZY=2;
/**
* @constant 
* @description Enum for YXZ rotation order
*/
GLGE.ROT_YXZ=3;
/**
* @constant 
* @description Enum for YZX rotation order
*/
GLGE.ROT_YZX=4;
/**
* @constant 
* @description Enum for ZXY rotation order
*/
GLGE.ROT_ZXY=5;
/**
* @constant 
* @description Enum for ZYX rotation order
*/
GLGE.ROT_ZYX=6;
/**
 * Creates a rotation matrix
 * @returns {Array} value an array GLGE.Vec or 3 paramters
 * @returns {GLGE.Mat} the rotation matrix
 */
GLGE.rotateMatrix=function(value,type) {
    var x;
    var y;
    var z;
	if(arguments.length>2){
		x=arguments[0];
		y=arguments[1];
		z=arguments[2];
		type=arguments[3];
	}
	else if(value.data){
		x=value.data[0];
		y=value.data[1];
		z=value.data[2];
	}
	else if(value instanceof Array){
		x=value[0];
		y=value[1];
		z=value[2];
	}
	if(!type) type=GLGE.ROT_XYZ;
	var cosx=Math.cos(x);
	var sinx=Math.sin(x);
	var cosy=Math.cos(y);
	var siny=Math.sin(y);
	var cosz=Math.cos(z);
	var sinz=Math.sin(z);
	var rotx=GLGE.Mat([1,0,0,0,0,cosx,-sinx,0,0,sinx,cosx,0,0,0,0,1]);
	var roty=GLGE.Mat([cosy,0,siny,0,0,1,0,0,-siny,0,cosy,0,0,0,0,1]);
	var rotz=GLGE.Mat([cosz,-sinz,0,0,sinz,cosz,0,0,0,0,1,0,0,0,0,1]);
	switch(type){
		case GLGE.ROT_XYZ:
			return GLGE.mulMat4(rotx,GLGE.mulMat4(roty,rotz));
			break;
		case GLGE.ROT_XZY:
			return GLGE.mulMat4(rotx,GLGE.mulMat4(rotz,roty));
			break;
		case GLGE.ROT_YXZ:
			return GLGE.mulMat4(roty,GLGE.mulMat4(rotx,rotz));
			break;
		case GLGE.ROT_YZX:
			return GLGE.mulMat4(roty,GLGE.mulMat4(rotz,rotx));
			break;
		case GLGE.ROT_ZXY:
			return GLGE.mulMat4(rotz,GLGE.mulMat4(rotx,roty));
			break;
		case GLGE.ROT_ZYX:
			return GLGE.mulMat4(rotz,GLGE.mulMat4(roty,rotx));
			break;
	}
}


GLGE.angleAxis=function(angle, axis) {
    var xmx,ymy,zmz,xmy,ymz,zmx,xms,yms,zms;
	axis=[axis[0],axis[1],axis[2],0];

        var x = axis[0];
        var y = axis[1];
        var z = axis[2];
	
	        
        var cos = Math.cos(angle);
        var cosi = 1.0 - cos;
	var sin = Math.sin(angle);
 
	xms = x * sin;yms = y * sin;zms = z * sin;
        xmx = x * x;ymy = y * y;zmz = z * z;
        xmy = x * y;ymz = y * z;zmx = z * x;
	
	var matrix = [(cosi * xmx) + cos,(cosi * xmy) - zms,(cosi * zmx) + yms,0,
			(cosi * xmy) + zms,(cosi * ymy) + cos,(cosi * ymz) - xms,0,
			(cosi * zmx) - yms,(cosi * ymz) + xms,(cosi * zmz) + cos,0,
			0,0,0,1];

        return GLGE.Mat(matrix);
};

GLGE.quatRotation=function(qx,qy,qz,qw){
	return GLGE.Mat([
	                    1 - 2*qy*qy - 2*qz*qz,2*qx*qy - 2*qz*qw,2*qx*qz + 2*qy*qw,0,
	                    2*qx*qy + 2*qz*qw,1 - 2*qx*qx - 2*qz*qz,2*qy*qz - 2*qx*qw,0,
	                    2*qx*qz - 2*qy*qw,2*qy*qz + 2*qx*qw,1 - 2*qx*qx - 2*qy*qy,0,
	                    0,0,0,1
	                ]);
};

GLGE.makeOrtho=function(left,right,bottom,top,near,far){
	var x = -(right+left)/(right-left);
	var y = -(top+bottom)/(top-bottom);
	var z = -(far+near)/(far-near);
    
        return GLGE.Mat([2/(right-left), 0, 0, x,
               0, 2/(top-bottom), 0, y,
               0, 0, -2/(far-near), z,
               0, 0, 0, 1]);
};

GLGE.makeFrustum=function(left,right,bottom,top,near,far){
	var x = 2*near/(right-left);
	var y = 2*near/(top-bottom);
	var a = (right+left)/(right-left);
	var b = (top+bottom)/(top-bottom);
	var c = -(far+near)/(far-near);
	var d = -2*far*near/(far-near);
	return GLGE.Mat([x, 0, a, 0,
		       0, y, b, 0,
		       0, 0, c, d,
		       0, 0, -1, 0]);
};

GLGE.makePerspective=function(fovy, aspect, near, far){
	var ymax = near * Math.tan(fovy * 0.00872664625972);
	var ymin = -ymax;
	var xmin = ymin * aspect;
	var xmax = ymax * aspect;
	return GLGE.makeFrustum(xmin, xmax, ymin, ymax, near, far);
};

GLGE.matrix2Scale=function(m){
	var m1=m[0];
	var m2=m[1];
	var m3=m[2];
	var m4=m[4];
	var m5=m[5];
	var m6=m[6];
	var m7=m[8];
	var m8=m[9];
	var m9=m[10];
	var scaleX=Math.sqrt(m1*m1+m2*m2+m3*m3);
	var scaleY=Math.sqrt(m4*m4+m5*m5+m6*m6);
	var scaleZ=Math.sqrt(m7*m7+m8*m8+m9*m9);
	return [scaleX,scaleY,scaleZ]
}


GLGE.rotationMatrix2Quat=function(m){
	var tr = m[0] + m[5] + m[10]+1.0;
	var S,x,y,z,w;

	if (tr > 0) { 
		S = 0.5/Math.sqrt(tr); 
		w = 0.25 / S;
		x = (m[9] - m[6]) * S;
		y = (m[2] - m[8]) * S; 
		z = (m[4] - m[1]) * S; 
	} else if ((m[0] > m[5])&&(m[0] > m[10])) { 
		S = Math.sqrt(1.0 + m[0] - m[5] - m[10]) * 2; 
		w = (m[9] - m[6]) / S;
		x = 0.25 / S;
		y = (m[1] + m[4]) / S; 
		z = (m[2] + m[8]) / S; 
	} else if (m[5] > m[10]) { 
		S = Math.sqrt(1.0 + m[5] - m[0] - m[10]) * 2;
		w = (m[2] - m[8]) / S;
		x = (m[1] + m[4]) / S; 
		y = 0.25 / S;
		z = (m[6] + m[9]) / S; 
	} else { 
		S = Math.sqrt(1.0 + m[10] - m[0] - m[5]) * 2; 
		w = (m[4] - m[1]) / S;
		x = (m[2] + m[8]) / S;
		y = (m[6] + m[9]) / S;
		z = 0.25 / S;
	}
	var N=Math.sqrt(x*x+y*y+z*z+w*w)
	
	return [x/N,y/N,z/N,w/N];
}



//returns plane as array [X,Y,Z,D]
GLGE.rayToPlane=function(origin,dir){
	var dirnorm=GLGE.toUnitVec3(dir);
	return [dirnorm[0],dirnorm[1],dirnorm[2],GLGE.dotVec3(origin,dirnorm)];
}

GLGE.rayIntersectPlane=function(origin,dir,plane){
	var planeN=[plane[0],plane[1],plane[2]];
	var planeD=plane[3];
	var vdir=GLGE.dotVec3(planeN,dir);
	if(vdir<=0){
		//ray in wrong direction
		return false;
	}
	var vo=-(GLGE.dotVec3(planeN,origin)+planeD);
	var t=vo/vdir;
	if(t<=0){
		return false;
	}
	return GLGE.addVec3(origin,GLGE.scaleVec3(dir,t));
}
//assumes perspective projection
GLGE.screenToDirection=function(x,y,width,height,proj){
	xcoord =  -( ( ( 2 * x ) / width ) - 1 ) / proj[0];
	ycoord =( ( ( 2 * y ) / height ) - 1 ) / proj[5];
	zcoord =  1;
	return GLGE.toUnitVec3([xcoord,ycoord,zcoord]);
}

GLGE.BoundingVolume=function(minX,maxX,minY,maxY,minZ,maxZ){
	var dims=[maxX-minX,maxY-minY,maxZ-minZ];
	this.dims=dims;
	this.center=[dims[0]/2+minX,dims[1]/2+minY,dims[2]/2+minZ];
}

//returns the center of the bounding area
GLGE.BoundingVolume.prototype.getCenter=function(matrix){
	return GLGE.mulMat4Vec4(matrix,this.center);
}

//returns box point
GLGE.BoundingVolume.prototype.getBoxPoint=function(matrix,point){
	var coord=[this.dims[0]/2*point[0]+this.center[0],this.dims[1]/2*point[1]+this.center[1],this.dims[2]/2*point[2]+this.center[2]];
	return GLGE.mulMat4Vec4(matrix,coord);
}

//returns the radius of a bounding sphere
GLGE.BoundingVolume.prototype.getSphereRadius=function(){
	return Math.pow((this.dims[0]*this.dims[0]+this.dims[1]*this.dims[1]+this.dims[2]*this.dims[2])/4,0.5);
}

//adds an additional bounding volume to resize the current and returns the result
GLGE.BoundingVolume.prototype.addBoundingVolume=function(vol){
	var minX=Math.min(this.center[0]-this.dims[0]/2,vol.center[0]-vol.dims[0]/2);
	var maxX=Math.max(this.center[0]+this.dims[0]/2,vol.center[0]+vol.dims[0]/2);
	var minY=Math.min(this.center[1]-this.dims[1]/2,vol.center[1]-vol.dims[1]/2);
	var maxY=Math.max(this.center[1]+this.dims[1]/2,vol.center[1]+vol.dims[1]/2);
	var minZ=Math.min(this.center[2]-this.dims[2]/2,vol.center[2]-vol.dims[2]/2);
	var maxZ=Math.max(this.center[2]+this.dims[2]/2,vol.center[2]+vol.dims[2]/2);
	var dims=[maxX-minX,maxY-minY,maxZ-minZ];
	this.dims=dims;
	this.center=[dims[0]/2+minX,dims[1]/2+minY,dims[2]/2+minZ];
}

//scales a volume based on a transform matrix
GLGE.BoundingVolume.prototype.applyMatrixScale=function(matrix){
	var scaleX=GLGE.lengthVec3([matrix[0],matrix[4],matrix[8]]);
	var scaleY=GLGE.lengthVec3([matrix[1],matrix[5],matrix[9]]);
	var scaleZ=GLGE.lengthVec3([matrix[2],matrix[6],matrix[10]]);
	var minX=(this.center[0]-this.dims[0]/2)*scaleX;
	var maxX=(this.center[0]+this.dims[0]/2)*scaleX;
	var minY=(this.center[1]-this.dims[1]/2)*scaleY;
	var maxY=(this.center[1]+this.dims[1]/2)*scaleY;
	var minZ=(this.center[2]-this.dims[2]/2)*scaleZ;
	var maxZ=(this.center[2]+this.dims[2]/2)*scaleZ;
	var dims=[maxX-minX,maxY-minY,maxZ-minZ];
	this.dims=dims;
	this.center=[dims[0]/2+minX,dims[1]/2+minY,dims[2]/2+minZ];
}

GLGE.BoundingVolume.prototype.clone=function(){
	var minX=this.center[0]-this.dims[0]/2;
	var maxX=this.center[0]+this.dims[0]/2;
	var minY=this.center[1]-this.dims[1]/2;
	var maxY=this.center[1]+this.dims[1]/2;
	var minZ=this.center[2]-this.dims[2]/2;
	var maxZ=this.center[2]+this.dims[2]/2;
	return new GLGE.BoundingVolume(minX,maxX,minY,maxY,minZ,maxZ);
}

GLGE.BoundingVolume.prototype.toString=function(){
	var minX=this.center[0]-this.dims[0]/2;
	var maxX=this.center[0]+this.dims[0]/2;
	var minY=this.center[1]-this.dims[1]/2;
	var maxY=this.center[1]+this.dims[1]/2;
	var minZ=this.center[2]-this.dims[2]/2;
	var maxZ=this.center[2]+this.dims[2]/2;
	return [minX,maxX,minY,maxY,minZ,maxZ].toString();
}


function GLGE_mathUnitTest() {
    var a=GLGE.Vec([1,2,3,4]);
    var b=GLGE.Vec4(GLGE.getVec4(a,3),
                    GLGE.get1basedVec4(a,3),
                    GLGE.getVec4(a,1),
                    GLGE.getVec4(a,0));
    var c=GLGE.identMatrix();
    var d=GLGE.mulMat4Vec4(c,b);
    if (GLGE.getVec4(d,0)!=4||
        GLGE.getVec4(d,1)!=3||
        GLGE.getVec4(d,2)!=2||
        GLGE.getVec4(d,3)!=1) {
        throw "Unit Test 1 failed MatVecMul "+d;
    }
    var m=GLGE.Mat4([3,4,5,0,.5,.75,0,0,.75,.5,0,0,.25,.25,1,1]);
    var m1=GLGE.Mat4([2,1,8,2,1,4,3,2,1,.5,6.5,2,8,3,1,.25]);
    var mm1=GLGE.mulMat4(m,m1);
    var am1=GLGE.Mat4([15,21.5,68.5,24,
                       1.75,3.5,6.25,2.5,
                       2,2.75,7.5,2.5,
                       9.75,4.75,10.25,3.25]);
    for (var i=0;i<4;++i) {
        for (var j=0;j<4;++j) {      
            var diff=GLGE.getMat4(mm1,i,j)-GLGE.getMat4(am1,i,j);
            if (diff<.000001&&diff>-.000001) {                

            }else {
                throw "Unit Test 1 failed Multiplication "+GLGE.getMat4(mm1,i,j)+" != "+GLGE.getMat4(am1,i,j);      
            }
        }
    }
    var inv = GLGE.inverseMat4(m);
    var k = GLGE.mulMat4(m,inv);
    var l = GLGE.mulMat4(inv,m);
    for (var i=0;i<4;++i) {
        for (var j=0;j<4;++j) {      
            var diff=GLGE.getMat4(k,i,j)-GLGE.getMat4(c,i,j);
            if (diff<.0001&&diff>-.0001) {                
            }else {
                throw "Unit Test 1 failed Inverse "+GLGE.getMat4(k,i,j)+" != "+GLGE.getMat4(c,i,j);   
            }
        }
    }
}
GLGE_mathUnitTest() ;


//Closure Export
GLGE["Vec3"]=GLGE.Vec3;
GLGE["Vec4"]=GLGE.Vec4;
GLGE["get1basedVec4"]=GLGE.get1basedVec4;
GLGE["get1basedVec3"]=GLGE.get1basedVec3;
GLGE["getVec4"]=GLGE.getVec4;
GLGE["getVec3"]=GLGE.getVec3;
GLGE["addVec4"]=GLGE.addVec4;
GLGE["addVec3"]=GLGE.addVec3;
GLGE["subVec4"]=GLGE.subVec4;
GLGE["subVec3"]=GLGE.subVec3;
GLGE["dotVec3"]=GLGE.dotVec3;
GLGE["dotVec4"]=GLGE.dotVec4;
GLGE["scaleVec4"]=GLGE.scaleVec4;
GLGE["scaleVec3"]=GLGE.scaleVec3;
GLGE["crossVec3"]=GLGE.crossVec3;
GLGE["toUnitVec3"]=GLGE.toUnitVec3;
GLGE["toUnitVec4"]=GLGE.toUnitVec4;
GLGE["lengthVec3"]=GLGE.lengthVec3;
GLGE["distanceVec3"]=GLGE.distanceVec3;
GLGE["lengthVec4"]=GLGE.lengthVec4;
GLGE["distanceVec4"]=GLGE.distanceVec4;
GLGE["angleVec3"]=GLGE.angleVec3;
GLGE["angleVec4"]=GLGE.angleVec4;
GLGE["Mat3"]=GLGE.Mat3;
GLGE["Mat"]=GLGE.Mat;
GLGE["Mat4"]=GLGE.Mat4;
GLGE["determinantMat4"]=GLGE.determinantMat4;
GLGE["inverseMat4"]=GLGE.inverseMat4;
GLGE["mulMat4Vec4"]=GLGE.mulMat4Vec4;
GLGE["scaleMat4"]=GLGE.scaleMat4;
GLGE["scaleInPlaceMat4"]=GLGE.scaleInPlaceMat4;
GLGE["addInPlaceMat4"]=GLGE.addInPlaceMat4;
GLGE["addMat4"]=GLGE.addMat4;
GLGE["subInPlaceMat4"]=GLGE.subInPlaceMat4;
GLGE["subMat4"]=GLGE.subMat4;
GLGE["mulMat4"]=GLGE.mulMat4;
GLGE["transposeInPlaceMat4"]=GLGE.transposeInPlaceMat4;
GLGE["transposeMat4"]=GLGE.transposeMat4;
GLGE["set1basedMat4"]=GLGE.set1basedMat4;
GLGE["setMat4"]=GLGE.setMat4;
GLGE["get1basedMat4"]=GLGE.get1basedMat4;
GLGE["getMat4"]=GLGE.getMat4;
GLGE["glDataMat4"]=GLGE.glDataMat4;
GLGE["identMatrix"]=GLGE.identMatrix;
GLGE["translateMatrix"]=GLGE.translateMatrix;
GLGE["scaleMatrix"]=GLGE.scaleMatrix;
GLGE["ROT_XYZ"]=GLGE.ROT_XYZ;
GLGE["ROT_XZY"]=GLGE.ROT_XZY;
GLGE["ROT_YXZ"]=GLGE.ROT_YXZ;
GLGE["ROT_YZX"]=GLGE.ROT_YZX;
GLGE["ROT_ZXY"]=GLGE.ROT_ZXY;
GLGE["ROT_ZYX"]=GLGE.ROT_ZYX;
GLGE["rotateMatrix"]=GLGE.rotateMatrix;
GLGE["angleAxis"]=GLGE.angleAxis;
GLGE["quatRotation"]=GLGE.quatRotation;
GLGE["makeOrtho"]=GLGE.makeOrtho;
GLGE["makeFrustum"]=GLGE.makeFrustum;
GLGE["makePerspective"]=GLGE.makePerspective;
GLGE["matrix2Scale"]=GLGE.matrix2Scale;
GLGE["rotationMatrix2Quat"]=GLGE.rotationMatrix2Quat;
GLGE["mat4gl"]=GLGE.mat4gl;


})(window["GLGE"]);
