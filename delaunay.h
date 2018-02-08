#pragma once

#include <cfloat>  
#include <cmath>  
#include <list>  

namespace Tercel
{
	struct Vector
	{
		double x, y;

		bool operator==(const Vector& v) const
		{
			return ( x == v.x && y == v.y );
		}
	};


	struct Circle
	{
		Vector center;  // ���S���W  
		double radius;  // ���a  
	};


	class Triangle
	{
	public:
		const Vector* p1, *p2, *p3;    // ���_���W  

	public:
		// ======================================    
		// �������̔���  
		// ======================================     
		bool operator==(const Triangle& t) const
		{
			return( *p1 == *t.p1 && *p2 == *t.p2 && *p3 == *t.p3 ||
				*p1 == *t.p2 && *p2 == *t.p3 && *p3 == *t.p1 ||
				*p1 == *t.p3 && *p2 == *t.p1 && *p3 == *t.p2 ||

				*p1 == *t.p3 && *p2 == *t.p2 && *p3 == *t.p1 ||
				*p1 == *t.p2 && *p2 == *t.p1 && *p3 == *t.p3 ||
				*p1 == *t.p1 && *p2 == *t.p3 && *p3 == *t.p2 );
		}

		// ======================================    
		// ���̎O�p�`�Ƌ��L�_������    
		// ======================================     
		bool hasCommonPoints(const Triangle& t)
		{
			return( *p1 == *t.p1 || *p1 == *t.p2 || *p1 == *t.p3 ||
				*p2 == *t.p1 || *p2 == *t.p2 || *p2 == *t.p3 ||
				*p3 == *t.p1 || *p3 == *t.p2 || *p3 == *t.p3 );
		}
	};


	class Delaunay2d
	{
	private:
		static void manageDuplicativeTriangles(std::list<Triangle>* newTriangleList,
			std::list<Triangle>* duplicativeTriangleList,
			const Triangle& t)
		{
			typedef std::list<Triangle>::iterator triIter;

			bool existsInNewTriangleList = false;
			for( triIter iter = newTriangleList->begin();
				iter != newTriangleList->end(); iter++ )
			{

				if( *iter == t ) {
					existsInNewTriangleList = true;
					bool existsInDuplicativeTriangleList = false;

					for( triIter iter2 = duplicativeTriangleList->begin();
						iter2 != duplicativeTriangleList->end(); iter2++ )
					{
						if( *iter2 == t )
						{
							existsInDuplicativeTriangleList = true;
							break;
						}

					}
					if( !existsInDuplicativeTriangleList )
					{
						duplicativeTriangleList->push_back(t);
					}
					break;
				}
			}
			if( !existsInNewTriangleList ) newTriangleList->push_back(t);
		}

	public:
		static void getDelaunayTriangles(const std::list<Vector>& vertexList,
			std::list<Triangle>* triangleList)
		{
			typedef std::list<Vector>::const_iterator cVtxIter;
			typedef std::list<Triangle>::iterator     triIter;

			Triangle hugeTriangle;
			{
				// ======================================    
				// �O���O�p�`�����    
				// ======================================    
				double maxX, maxY; maxX = maxY = DBL_MIN;
				double minX, minY; minX = minY = DBL_MAX;
				for( cVtxIter it = vertexList.begin(); it != vertexList.end(); ++it )
				{
					double x = it->x;
					double y = it->y;
					if( maxX < x ) maxX = x; if( minX > x ) minX = x;
					if( maxY < y ) maxY = y; if( minY > y ) minY = y;
				}

				// ���ׂĂ̓_���܂����`�̊O�ډ~  
				double centerX = ( maxX - minX ) * 0.5;          // ���Sx���W  
				double centerY = ( maxY - minY ) * 0.5;          // ���Sy���W  

				double dX = maxX - centerX;
				double dY = maxY - centerY;
				double radius = sqrt(dX * dX + dY * dY) + 1.0;  // ���a  

				Vector* p1 = new Vector;    // �������m�ہi266�s�ڂŉ���j  
				p1->x = centerX - sqrt(3.0) * radius;
				p1->y = centerY - radius;

				Vector* p2 = new Vector;    // �������m�ہi267�s�ڂŉ���j  
				p2->x = centerX + sqrt(3.0) * radius;
				p2->y = centerY - radius;

				Vector* p3 = new Vector;    // �������m�ہi268�s�ڂŉ���j  
				p3->x = centerX;
				p3->y = centerY + 2.0 * radius;

				hugeTriangle.p1 = p1;
				hugeTriangle.p2 = p2;
				hugeTriangle.p3 = p3;
			}

			triangleList->push_back(hugeTriangle);

			// --------------------------------------    
			// �_�𒀎��Y�����A�����I�ɎO�p�������s��    
			// --------------------------------------    
			for( cVtxIter vIter = vertexList.begin(); vIter != vertexList.end(); ++vIter )
			{
				const Vector* p = &( *vIter );

				// --------------------------------------    
				// �ǉ����̎O�p�`��ێ�����ꎞ���X�g    
				// --------------------------------------    
				std::list<Triangle> newTriangleList;          // �V�K�������ꂽ�O�p�`                  
				std::list<Triangle> duplicativeTriangleList;  // �d�����X�g  


															  // --------------------------------------    
															  // ���݂̎O�p�`�Z�b�g����v�f��������o���āA    
															  // �^����ꂽ�_���e�X�̎O�p�`�̊O�ډ~�̒��Ɋ܂܂�邩�ǂ�������    
															  // --------------------------------------    
				for( triIter tIter = triangleList->begin(); tIter != triangleList->end(); )
				{
					// �O�p�`�Z�b�g����O�p�`����肾���āc  
					Triangle t = *tIter;

					// ���̊O�ډ~�����߂�B    
					Circle   c;
					{
						// �O�p�`�̊e���_���W�� (x1, y1), (x2, y2), (x3, y3) �Ƃ��A    
						// ���̊O�ډ~�̒��S���W�� (x, y) �Ƃ���ƁA    
						//     (x - x1) * (x - x1) + (y - y1) * (y - y1)    
						//   = (x - x2) * (x - x2) + (y - y2) * (y - y2)    
						//   = (x - x3) * (x - x3) + (y - y3) * (y - y3)    
						// ���A�ȉ��̎������藧��    
						//    
						// x = { (y3 - y1) * (x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1)    
						//     + (y1 - y2) * (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1)} / c    
						//    
						// y = { (x1 - x3) * (x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1)    
						//     + (x2 - x1) * (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1)} / c    
						//    
						// �������A    
						//   c = 2 * {(x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1)}   

						double x1 = t.p1->x;    double y1 = t.p1->y;
						double x2 = t.p2->x;    double y2 = t.p2->y;
						double x3 = t.p3->x;    double y3 = t.p3->y;

						double m = 2.0 * ( ( x2 - x1 ) * ( y3 - y1 ) - ( y2 - y1 ) * ( x3 - x1 ) );
						double x = ( ( y3 - y1 ) * ( x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1 )
							+ ( y1 - y2 ) * ( x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1 ) ) / m;
						double y = ( ( x1 - x3 ) * ( x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1 )
							+ ( x2 - x1 ) * ( x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1 ) ) / m;

						c.center.x = x;
						c.center.y = y;

						// �O�ډ~�̔��a r �́A���a����O�p�`�̔C�ӂ̒��_�܂ł̋����ɓ�����   
						double dx = t.p1->x - x;
						double dy = t.p1->y - y;
						double radius = sqrt(dx * dx + dy * dy);

						c.radius = radius;
					}

					double dx = c.center.x - p->x;
					double dy = c.center.y - p->y;
					double dist = sqrt(dx * dx + dy * dy);

					// ======================================    
					// �ꎞ���X�g���g���ďd������    
					// ======================================    
					if( dist < c.radius )
					{
						// �ĕ���  

						Triangle t1;
						t1.p1 = p; t1.p2 = t.p1; t1.p3 = t.p2;
						manageDuplicativeTriangles(&newTriangleList, &duplicativeTriangleList, t1);

						Triangle t2;
						t2.p1 = p; t2.p2 = t.p2; t2.p3 = t.p3;
						manageDuplicativeTriangles(&newTriangleList, &duplicativeTriangleList, t2);

						Triangle t3;
						t3.p1 = p; t3.p2 = t.p3; t3.p3 = t.p1;
						manageDuplicativeTriangles(&newTriangleList, &duplicativeTriangleList, t3);

						tIter = triangleList->erase(tIter);
					}
					else ++tIter;
				}

				// --------------------------------------    
				// �ꎞ���X�g�̂����A�d���̂Ȃ����̂��O�p�`���X�g�ɒǉ�     
				// --------------------------------------    
				for( triIter iter = newTriangleList.begin();
					iter != newTriangleList.end(); ++iter )
				{
					bool exists = false;
					for( triIter iter2 = duplicativeTriangleList.begin();
						iter2 != duplicativeTriangleList.end(); ++iter2 )
					{
						if( *iter == *iter2 )
						{
							exists = true;
							break;
						}
					}
					if( !exists ) triangleList->push_back(*iter);
				}
			}

			// �Ō�ɁA�O���O�p�`�̒��_���폜  
			for( triIter tIter = triangleList->begin(); tIter != triangleList->end(); )
			{
				if( hugeTriangle.hasCommonPoints(*tIter) ) tIter = triangleList->erase(tIter);
				else ++tIter;
			}

			// ����O�p�`�̒��_�����  
			delete hugeTriangle.p1;
			delete hugeTriangle.p2;
			delete hugeTriangle.p3;
		}
	};
}
