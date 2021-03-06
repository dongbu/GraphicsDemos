#ifndef CAMERA_H
#define CAMERA_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT   0x0600
#endif

#include "d3dx9.h"
#include "ArcBall.h"

class Camera
{
public:
	Camera(void);
	~Camera(void);

public:
	void Reset() ;
	void OnFrameMove() ;
	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
	void SetViewParams(D3DXVECTOR3* pvEyePt, D3DXVECTOR3* pvLookatPt, D3DXVECTOR3* pvUp);
	void SetProjParams(float fFOV, float fAspect, float fNearPlane, float fFarPlane) ;
	void SetWindow(int nWidth, int nHeight, float fArcballRadius = 0.9f) ;
	const D3DXMATRIX* GetWorldMatrix() const ;
	const D3DXMATRIX* GetViewMatrix() const ;
	const D3DXMATRIX* GetProjMatrix() const ;
	void SetWorldMatrix(const D3DXMATRIX *matWorld) ;
	void SetViewMatrix(const D3DXMATRIX* matView) ;
	void SetProjectionMatrix(const D3DXMATRIX* matProj) ;

private:
	float m_fRadius;				// Distance from the camera to model 
	float m_fDefaultRadius;			// Distance from the camera to model 
	int m_nMouseWheelDelta;			// Amount of middle wheel scroll (+/-)
	D3DXVECTOR3 m_vModelCenter ;	// Model center
	
	D3DXVECTOR3 m_vEyePt ;			// Eye position
	D3DXVECTOR3 m_vLookatPt ;		// Look at position
	D3DXVECTOR3 m_vUp ;				// Up vector

	float m_fFOV;					// Field of view
	float m_fAspect;				// Aspect ratio
	float m_fNearPlane;				// Near plane
	float m_fFarPlane;				// Far plane
	
	// 这里是不是有些重复呢，稍后仔细斟酌一下
	D3DXMATRIX m_matView ;			// Camera View matrix
	D3DXMATRIX m_matProj ;			// Camera Projection matrix
	D3DXMATRIX m_matWorld ;			// World matrix of model
	D3DXMATRIX m_matModelRotate ;	// Rotation matrix of model
	D3DXMATRIX m_matModelRotLast ;	// model last rotatin matrix
	D3DXMATRIX m_matCameraRotLast ;	// Camera last rotation matrix

	ArcBall m_WorldArcBall ;		// World arc ball 
	ArcBall m_ViewArcBall ;			// View arc ball
};

#endif // end __CAMERA_H__
