#pragma once
#include <GameObject.h>

#include "../Delegate.h"

class SpriteFont;

class Button : public GameObject
{
public:
	explicit Button(const std::wstring& text, const std::wstring& fontPath, const DirectX::XMFLOAT4& color = {1.f, 1.f, 1.f, 1.f});

	void SetColor(DirectX::XMFLOAT4 color) { m_Color = std::move(color); }
	void SetHoverColor(DirectX::XMFLOAT4 color) { m_HoverColor = std::move(color); }
	void SetSelectedColor(DirectX::XMFLOAT4 color) { m_SelectedColor = std::move(color); }

	void RegisterOnHoverEvent(const std::string& evtName, std::function<void()> callback) { OnHover.Register(evtName, callback); }
	void RegisterOnSelectedEvent(const std::string& evtName, std::function<void()> callback) { OnSelected.Register(evtName, callback); }
	void RegisterOnClickEvent(const std::string& evtName, std::function<void()> callback) { OnClick.Register(evtName, callback); }

	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;

private:
	DirectX::XMFLOAT4 m_Color;
	DirectX::XMFLOAT4 m_HoverColor;
	DirectX::XMFLOAT4 m_SelectedColor;
	DirectX::XMFLOAT2 m_TextSize;
	std::wstring m_Text;
	SpriteFont* m_pFont;

	Event<> OnHover;
	Event<> OnSelected;
	Event<> OnClick;

	bool m_IsHovered;
};

