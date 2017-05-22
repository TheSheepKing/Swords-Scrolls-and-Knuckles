#ifndef RENDERER_HPP
# define RENDERER_HPP

# include <memory>
# include <OgreSceneManager.h>
# include <OgreCamera.h>
# include <OgreViewport.h>
# include "Scene.hpp"

class Renderer
{
  private:
    Ogre::SceneManager *scenemgr;
    Ogre::Camera *camera;
    Ogre::Viewport *viewport;
    std::unique_ptr<Scene> scene;

  public:
    Renderer(void) = delete;
    Renderer(Game &);
    Renderer(Renderer const &) = delete;
    Renderer(Renderer &&) = delete;
    ~Renderer(void) = default;
    Renderer &operator=(Renderer const &) = delete;
    Renderer &operator=(Renderer &&) = delete;

    /// Switch the current scene to the given one.
    void switchScene(Game &game, std::unique_ptr<Scene> &&);

    /// Returns the scene manager
    Ogre::SceneManager &getSceneManager(void);
    Ogre::SceneManager const &getSceneManager(void) const;

    /// Returns the camera
    Ogre::Camera &getCamera(void);
    Ogre::Camera const &getCamera(void) const;


    /// Returns the current scene
    std::unique_ptr<Scene> &getScene(void);
    std::unique_ptr<Scene> const &getScene(void) const;
};

#endif // !RENDERER_HPP
