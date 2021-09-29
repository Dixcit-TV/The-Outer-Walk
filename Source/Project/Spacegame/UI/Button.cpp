#include "stdafx.h"
#include "Button.h"

#include "ContentManager.h"
#include "TextRenderer.h"
#include "SpriteFont.h"
#include "TransformComponent.h"

Button::Button(const std::wstring& text, const std::wstring& fontPath, const DirectX::XMFLOAT4& color)
	: GameObject()
	, m_Color(color)
	, m_HoverColor(color)
	, m_SelectedColor(color)
	, m_TextSize(0.f, 0.f)
	, m_Text(text)
	, m_pFont(ContentManager::Load<SpriteFont>(fontPath))
	, OnHover()
	, OnSelected()
	, OnClick()
	, m_IsHovered(false)
{}

void Button::Initialize(const GameContext&)
{
	m_TextSize = m_pFont->GetTextSize(m_Text);
}

void Button::Update(const GameContext&)
{
	const POINT mousePos{ InputManager::GetMousePosition() };
	const bool mouseClick{ InputManager::IsMouseButtonDown(VK_LBUTTON) && !InputManager::IsMouseButtonDown(VK_LBUTTON, true) };
	DirectX::XMFLOAT4 color{ m_Color };
	const DirectX::XMFLOAT3& worldPos{ GetTransform()->GetWorldPosition() };
	const DirectX::XMFLOAT2& screenPos{ worldPos.x, worldPos.y };
	const DirectX::XMFLOAT2 position(screenPos.x - m_TextSize.x * 0.5f, screenPos.y - m_TextSize.y * 0.5f);
	
	if (mousePos.x >= position.x && mousePos.x <= position.x + m_TextSize.x
		&& mousePos.y >= position.y && mousePos.y <= position.y + m_TextSize.y)
	{
		if (!m_IsHovered)
			OnHover.Invoke();

		m_IsHovered = true;

		color = m_HoverColor;

		if (mouseClick)
			OnClick.Invoke();
	}
	else
		m_IsHovered = false;

	TextRenderer::GetInstance()->DrawText(m_pFont, m_Text.c_str(), position, color);
}