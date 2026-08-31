#include "CollisionManager.h"
#include "Logger.h"

std::unique_ptr<CollisionManager> CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = std::make_unique<CollisionManager>(ConstructorKey());
	}
	return instance_.get();
}

void CollisionManager::Finalize()
{
	Clear();
	instance_.reset();
}

void CollisionManager::AddCollider(Collider* collider)
{
	colliders_.push_back(collider);
}

void CollisionManager::Clear()
{
	colliders_.clear();
}

void CollisionManager::CheckAllCollisions()
{
    for (auto itrA = colliders_.begin(); itrA != colliders_.end(); ++itrA) 
    {
        auto itrB = itrA;
        ++itrB;

        for (; itrB != colliders_.end(); ++itrB) 
        {
            Collider* a = *itrA;
            Collider* b = *itrB;

			if ((a->GetAttribute() & b->GetMask()) == 0 ||
				(b->GetAttribute() & a->GetMask()) == 0)
			{
				continue;
			}

            bool isColliding = false;
            if (a->GetShape() == ColliderShape::Sphere &&
                b->GetShape() == ColliderShape::Sphere)
            {
                const Sphere sphereA{ a->GetWorldPosition(), a->GetRadius() };
                const Sphere sphereB{ b->GetWorldPosition(), b->GetRadius() };
                isColliding = IsCollision(sphereA, sphereB);
            }
            else if (a->GetShape() == ColliderShape::AABB &&
                     b->GetShape() == ColliderShape::AABB)
            {
                isColliding = IsCollision(a->GetAABB(), b->GetAABB());
            }
            else if (a->GetShape() == ColliderShape::AABB)
            {
                const Sphere sphereB{ b->GetWorldPosition(), b->GetRadius() };
                isColliding = IsCollision(a->GetAABB(), sphereB);
            }
            else
            {
                const Sphere sphereA{ a->GetWorldPosition(), a->GetRadius() };
                isColliding = IsCollision(b->GetAABB(), sphereA);
            }

            if (isColliding)
            {
                a->OnCollision(b);
                b->OnCollision(a);
            }
        }
    }
}

