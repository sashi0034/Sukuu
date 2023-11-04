#pragma once

namespace Util
{
	class ActorContainer;

	class ActorBase
	{
	public:
		ActorBase();
		virtual ~ActorBase() = default;
		virtual void Update();
		virtual double OrderPriority() const;

		void Kill();
		bool IsDead() const;

		bool HasChildren() const;
		[[nodiscard]] ActorContainer& AsParent();

		friend class ActorView;
		friend class ActorWeak;

	private:
		struct Impl;
		std::shared_ptr<Impl> p_impl;
	};

	// 非所有権アクター
	class ActorView
	{
	public:
		ActorView(const ActorBase& actor);

		bool IsDead() const;

		[[nodiscard]] ActorContainer& AsParent();

	private:
		ActorBase::Impl* p_impl{};
	};

	// 弱参照アクター
	class ActorWeak
	{
	public:
		ActorWeak() = default;
		ActorWeak(const ActorBase& actor);

		void Kill();
		bool IsDead() const;

	private:
		std::weak_ptr<ActorBase::Impl> p_impl{};
	};
}
