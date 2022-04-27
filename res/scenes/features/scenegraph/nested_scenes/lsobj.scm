
(define childSceneId (load-scene "./res/scenes/features/scenegraph/nested_scenes/childscene.rawscene"
  (list
    ;(list "(dialog" "anchor" ".PARENT/)menuitem_0")
  )
  "testname"
))
(define childPlatformName (string-append "." (number->string childSceneId) "/platform"))

(format #t "child platform: ~a\n" childPlatformName)
(define testSceneId (load-scene "./res/scenes/features/scenegraph/nested_scenes/testscene.rawscene"
  (list
    (list ">testscenecamera" "lookat" childPlatformName)
  )
))

(define currsceneid (list-sceneid (gameobj-id mainobj)))
(define targetOtherSceneObject (string-append "." (number->string testSceneId) "/testobject"))
(define targetOtherSceneCamera (string-append "." (number->string testSceneId) "/>testscenecamera"))

(define (onMouse button action mods)
  (define testnameSceneId (lsscene-name "testname"))
  (define testnameSceneIdRoot (if testnameSceneId (scene-rootid testnameSceneId) #f))
  (define currRoot (scene-rootid currsceneid))
  (define gameObjToRemove (lsobj-name targetOtherSceneObject))
  (define objToRemove (if gameObjToRemove (gameobj-id gameObjToRemove) #f))
  (if (and testnameSceneId currRoot) (make-parent testnameSceneIdRoot currRoot))
  ;(if testnameSceneId (unload-scene testnameSceneId))

  (format #t "\n")
  (if (and (equal? action 1) (equal? button 0))
    (begin
      (format #t "LSOBJ CHILD SCENES\n==================================\n")
      (format #t "button is: ~a\n" button)
      (format #t "lsobj script: scene id: ~a\n" currsceneid)
      (format #t "parent scene id: ~a\n" (parent-scene currsceneid))
      (format #t "child scene id: ~a\n" (child-scenes currsceneid))
      (format #t "lsobj script: BASIC: id is: ~a\n" (gameobj-id (lsobj-name "someotherobject")))
      (format #t "loaded scene id is: ~a\n" (number->string testnameSceneId))
      (format #t "root id is: ~a\n" (number->string testnameSceneIdRoot))

      (format #t "LSOBJ SCENE PREFIX\n==================================\n")
      (format #t "target obj: ~a\n" targetOtherSceneObject)
      (format #t "obj name: ~a\n" (lsobj-name targetOtherSceneObject))
      (if objToRemove (rm-obj objToRemove))
    )
  )
  (if (and (equal? action 1) (equal? button 1))
    (begin
      (format #t "setting to camera: ~a\n" targetOtherSceneCamera)
      (set-camera (gameobj-id (lsobj-name targetOtherSceneCamera)))
    )
  )
)

