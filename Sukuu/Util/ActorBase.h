#pragma once

namespace Util
{
	class ActorContainer;

	struct IActor
	{
		virtual ~IActor() = default;
		virtual void Update() = 0;
		virtual double OrderPriority() const = 0;
		virtual bool IsActive() const = 0;
		virtual void Kill() = 0;
		virtual bool IsDead() const = 0;
	};

	class ActorBase : public IActor
	{
	public:
		ActorBase();
		~ActorBase() override = default;
		void Update() override;
		double OrderPriority() const override;

		void SetActive(bool isActive);
		bool IsActive() const override;

		void Kill() override;
		bool IsDead() const override;

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
