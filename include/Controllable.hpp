#ifndef CONTROLLABLE_HPP
# define CONTROLLABLE_HPP

# include "Fixture.hpp"

class SaveState;
class Logic;

class Controllable : public Fixture
{
private:
  Vect<2u, double> input;
  Vect<2u, double> dir;
  Vect<2u, double> targetDir;
  unsigned int stun;
  bool locked;
  unsigned int health;
  unsigned int maxHealth;

public:
  unsigned int invulnerable;
  unsigned int dePopCounter;

  template<class... PARAMS>
  constexpr Controllable(unsigned int health, PARAMS &&... params)
  : Fixture{std::forward<PARAMS>(params)..., Vect<2u, double>{0.0, 0.0}, true}
    , input{0.0, 0.0}
    , dir{0.0, 1.0}
    , targetDir(dir)
    , stun(0)
    , locked(false)
    , health(health)
    , maxHealth(health)
    , invulnerable(false)
    , dePopCounter(0u)
  {
  }

  Controllable() = default;
  constexpr void update(Logic &logic);

  constexpr bool isDead() const
  {
    return health == 0;
  }

  constexpr bool shouldBeRemoved() const
  {
    return isDead() && dePopCounter > 600u;
  }

  constexpr bool doCollision() const
  {
    return !isDead();
  }

  constexpr void knockback(Vect<2u, double> speed, unsigned int stun)
  {
    if (!invulnerable)
      {
	targetDir = -speed.normalized();
	this->speed = speed;
	this->stun = stun;
      }
  }

  constexpr int getMaxHealth(void) const {
    return maxHealth;
  }

  constexpr int getHealth(void) const {
    return health;
  }

  constexpr void takeDamage(unsigned int damage)
  {
    if (!invulnerable)
      {
	if (damage < health)
	  health -= damage;
	else
	  health = 0;
	invulnerable = 10;
      }
  }

  void heal(unsigned int amount)
  {
    health += amount;
    if (health > maxHealth)
      health = maxHealth;
  }

  constexpr bool isWalking() const
  {
    return !stun && !input.equals({0.0, 0.0});
  }

  constexpr bool isStun() const
  {
    return stun;
  }

  // setter only.
  constexpr void setInput(Vect<2u, double> input)
  {
    if (!stun && input.length2() > 0)
      targetDir = input.unsafeNormalized();
    this->input = input;
  }

  constexpr void dash(double speed, unsigned int time)
  {
    this->speed = input * speed;
    this->stun = time;
    this->invulnerable = time;
  }

  // setter only.
  constexpr void setLocked(bool locked)
  {
    this->locked = locked;
  }

  constexpr void setInputPy(double a, double b)
  {
    setInput({a, b});
  }

  constexpr Vect<2u, double> getDir() const
  {
    return dir;
  }

  constexpr void setDir(Vect<2u, double> dir)
  {
    this->dir = dir;
  }

  void   serialize(SaveState &state) const;
  void   unserialize(LoadGame &);
};

#endif // !CONTROLLABLE_HPP
