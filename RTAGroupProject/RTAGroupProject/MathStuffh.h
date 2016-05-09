#pragma once
#include<cmath>
struct Matrix
{
	float mat[4][4];
};
struct Vertex
{
	float x, y, z, w, u, v;

	unsigned int color;
};
struct SIMPLE_VERTEX
{
	float x, y, z;
	float u, v, w;
	float n, r, m;

};
struct SEND_TO_OBJECT
{
	Matrix WorldMat;
};
struct SEND_TO_SCENE
{
	Matrix ViewMatrix;
	Matrix ProjectionMatrix;
};
struct SEND_TO_VRAM
{
	float constantColor[4];
	float constantOffset[2];
	float padding[2];
};
struct tVertex
{
	float fX, fY, fZ;
	float fNX, fNY, fNZ;
	float fU, fV;
};
struct tTriangle
{
	unsigned int uIndices[3];
};
Vertex VertexMultMatrix(Vertex vec, Matrix mat)
{
	Vertex tempvec;
	tempvec.x = (vec.x * mat.mat[0][0]) + (vec.y* mat.mat[1][0]) + (vec.z * mat.mat[2][0] + (vec.w * mat.mat[3][0]));
	tempvec.y = (vec.x * mat.mat[0][1]) + (vec.y* mat.mat[1][1]) + (vec.z * mat.mat[2][1] + (vec.w * mat.mat[3][1]));
	tempvec.z = (vec.x * mat.mat[0][2]) + (vec.y* mat.mat[1][2]) + (vec.z * mat.mat[2][2] + (vec.w * mat.mat[3][2]));
	tempvec.w = (vec.x * mat.mat[0][3]) + (vec.y* mat.mat[1][3]) + (vec.z * mat.mat[2][3] + (vec.w * mat.mat[3][3]));
	tempvec.u = vec.u;
	tempvec.v = vec.v;

	return tempvec;
}
void SpecialCaseMatInverse(Matrix& mat)
{
	Matrix temp = mat;
	temp.mat[1][0] = mat.mat[0][1];
	temp.mat[2][0] = mat.mat[0][2];
	temp.mat[0][1] = mat.mat[1][0];
	temp.mat[2][1] = mat.mat[1][2];
	temp.mat[0][2] = mat.mat[2][0];
	temp.mat[1][2] = mat.mat[2][1];


	temp.mat[2][3] = 0;
	temp.mat[1][3] = 0;
	temp.mat[0][3] = 0;
	temp.mat[3][0] = 0;
	temp.mat[3][1] = 0;
	temp.mat[3][2] = 0;
	temp.mat[3][3] = 1;


	Vertex tempvert;
	tempvert.x = mat.mat[3][0];
	tempvert.y = mat.mat[3][1];
	tempvert.z = mat.mat[3][2];
	tempvert.w = 0;

	tempvert = VertexMultMatrix(tempvert, temp);

	tempvert.x = -tempvert.x;
	tempvert.y = -tempvert.y;
	tempvert.z = -tempvert.z;

	mat = temp;

	mat.mat[3][0] = tempvert.x;
	mat.mat[3][1] = tempvert.y;
	mat.mat[3][2] = tempvert.z;
	mat.mat[3][3] = 1;
}
Matrix Identity()
{
	Matrix RotateMat;
	RotateMat.mat[0][0] = 1;
	RotateMat.mat[1][0] = 0;
	RotateMat.mat[2][0] = 0;
	RotateMat.mat[3][0] = 0;
	RotateMat.mat[0][1] = 0;
	RotateMat.mat[1][1] = 1;
	RotateMat.mat[2][1] = 0;
	RotateMat.mat[3][1] = 0;
	RotateMat.mat[0][2] = 0;
	RotateMat.mat[1][2] = 0;
	RotateMat.mat[2][2] = 1;
	RotateMat.mat[3][2] = 0;
	RotateMat.mat[0][3] = 0;
	RotateMat.mat[1][3] = 0;
	RotateMat.mat[2][3] = 0;
	RotateMat.mat[3][3] = 1;

	return RotateMat;
}

Matrix DirectXWorldMat()
{
	Matrix RotateMat;
	RotateMat.mat[0][0] = 1;
	RotateMat.mat[1][0] = 0;
	RotateMat.mat[2][0] = 0;
	RotateMat.mat[3][0] = 0;
	RotateMat.mat[0][1] = 0;
	RotateMat.mat[1][1] = 1;
	RotateMat.mat[2][1] = 0;
	RotateMat.mat[3][1] = 0;
	RotateMat.mat[0][2] = 0;
	RotateMat.mat[1][2] = 0;
	RotateMat.mat[2][2] = 1;
	RotateMat.mat[3][2] = 5;
	RotateMat.mat[0][3] = 0;
	RotateMat.mat[1][3] = 0;
	RotateMat.mat[2][3] = 0;
	RotateMat.mat[3][3] = 1;

	return RotateMat;
}
Matrix MatrixMultMatrix(Matrix mat1, Matrix mat2)
{
	Matrix tempmat;
	tempmat.mat[0][0] = ((mat1.mat[0][0] * mat2.mat[0][0]) + (mat1.mat[0][1] * mat2.mat[1][0]) + (mat1.mat[0][2] * mat2.mat[2][0]) + (mat1.mat[0][3] * mat2.mat[3][0]));
	tempmat.mat[0][1] = ((mat1.mat[0][0] * mat2.mat[0][1]) + (mat1.mat[0][1] * mat2.mat[1][1]) + (mat1.mat[0][2] * mat2.mat[2][1]) + (mat1.mat[0][3] * mat2.mat[3][1]));
	tempmat.mat[0][2] = ((mat1.mat[0][0] * mat2.mat[0][2]) + (mat1.mat[0][1] * mat2.mat[1][2]) + (mat1.mat[0][2] * mat2.mat[2][2]) + (mat1.mat[0][3] * mat2.mat[3][2]));
	tempmat.mat[0][3] = ((mat1.mat[0][0] * mat2.mat[0][3]) + (mat1.mat[0][1] * mat2.mat[1][3]) + (mat1.mat[0][2] * mat2.mat[2][3]) + (mat1.mat[0][3] * mat2.mat[3][3]));

	tempmat.mat[1][0] = ((mat1.mat[1][0] * mat2.mat[0][0]) + (mat1.mat[1][1] * mat2.mat[1][0]) + (mat1.mat[1][2] * mat2.mat[2][0]) + (mat1.mat[1][3] * mat2.mat[3][0]));
	tempmat.mat[1][1] = ((mat1.mat[1][0] * mat2.mat[0][1]) + (mat1.mat[1][1] * mat2.mat[1][1]) + (mat1.mat[1][2] * mat2.mat[2][1]) + (mat1.mat[1][3] * mat2.mat[3][1]));
	tempmat.mat[1][2] = ((mat1.mat[1][0] * mat2.mat[0][2]) + (mat1.mat[1][1] * mat2.mat[1][2]) + (mat1.mat[1][2] * mat2.mat[2][2]) + (mat1.mat[1][3] * mat2.mat[3][2]));
	tempmat.mat[1][3] = ((mat1.mat[1][0] * mat2.mat[0][3]) + (mat1.mat[1][1] * mat2.mat[1][3]) + (mat1.mat[1][2] * mat2.mat[2][3]) + (mat1.mat[1][3] * mat2.mat[3][3]));

	tempmat.mat[2][0] = ((mat1.mat[2][0] * mat2.mat[0][0]) + (mat1.mat[2][1] * mat2.mat[1][0]) + (mat1.mat[2][2] * mat2.mat[2][0]) + (mat1.mat[2][3] * mat2.mat[3][0]));
	tempmat.mat[2][1] = ((mat1.mat[2][0] * mat2.mat[0][1]) + (mat1.mat[2][1] * mat2.mat[1][1]) + (mat1.mat[2][2] * mat2.mat[2][1]) + (mat1.mat[2][3] * mat2.mat[3][1]));
	tempmat.mat[2][2] = ((mat1.mat[2][0] * mat2.mat[0][2]) + (mat1.mat[2][1] * mat2.mat[1][2]) + (mat1.mat[2][2] * mat2.mat[2][2]) + (mat1.mat[2][3] * mat2.mat[3][2]));
	tempmat.mat[2][3] = ((mat1.mat[2][0] * mat2.mat[0][3]) + (mat1.mat[2][1] * mat2.mat[1][3]) + (mat1.mat[2][2] * mat2.mat[2][3]) + (mat1.mat[2][3] * mat2.mat[3][3]));

	tempmat.mat[3][0] = ((mat1.mat[3][0] * mat2.mat[0][0]) + (mat1.mat[3][1] * mat2.mat[1][0]) + (mat1.mat[3][2] * mat2.mat[2][0]) + (mat1.mat[3][3] * mat2.mat[3][0]));
	tempmat.mat[3][1] = ((mat1.mat[3][0] * mat2.mat[0][1]) + (mat1.mat[3][1] * mat2.mat[1][1]) + (mat1.mat[3][2] * mat2.mat[2][1]) + (mat1.mat[3][3] * mat2.mat[3][1]));
	tempmat.mat[3][2] = ((mat1.mat[3][0] * mat2.mat[0][2]) + (mat1.mat[3][1] * mat2.mat[1][2]) + (mat1.mat[3][2] * mat2.mat[2][2]) + (mat1.mat[3][3] * mat2.mat[3][2]));
	tempmat.mat[3][3] = ((mat1.mat[3][0] * mat2.mat[0][3]) + (mat1.mat[3][1] * mat2.mat[1][3]) + (mat1.mat[3][2] * mat2.mat[2][3]) + (mat1.mat[3][3] * mat2.mat[3][3]));

	return tempmat;
}
Matrix RotationX(float radian)
{
	Matrix RotateMat;
	RotateMat.mat[0][0] = 1;
	RotateMat.mat[1][0] = 0;
	RotateMat.mat[2][0] = 0;
	RotateMat.mat[3][0] = 0;
	RotateMat.mat[0][1] = 0;
	RotateMat.mat[1][1] = cos(radian);
	RotateMat.mat[2][1] = sin(radian);
	RotateMat.mat[3][1] = 0;
	RotateMat.mat[0][2] = 0;
	RotateMat.mat[1][2] = -sin(radian);
	RotateMat.mat[2][2] = cos(radian);
	RotateMat.mat[3][2] = 0;
	RotateMat.mat[0][3] = 0;
	RotateMat.mat[1][3] = 0;
	RotateMat.mat[2][3] = 0;
	RotateMat.mat[3][3] = 1;

	return RotateMat;
}

Matrix RotationY(float radian)
{
	Matrix RotateMat;
	RotateMat.mat[0][0] = cos(radian);
	RotateMat.mat[1][0] = 0;
	RotateMat.mat[2][0] = -sin(radian);
	RotateMat.mat[3][0] = 0;
	RotateMat.mat[0][1] = 0;
	RotateMat.mat[1][1] = 1;
	RotateMat.mat[2][1] = 0;
	RotateMat.mat[3][1] = 0;
	RotateMat.mat[0][2] = sin(radian);
	RotateMat.mat[1][2] = 0;
	RotateMat.mat[2][2] = cos(radian);
	RotateMat.mat[3][2] = 0;
	RotateMat.mat[0][3] = 0;
	RotateMat.mat[1][3] = 0;
	RotateMat.mat[2][3] = 0;
	RotateMat.mat[3][3] = 1;

	return RotateMat;
}