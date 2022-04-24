
(load-scene "./res/scenes/features/scenegraph/nested_scenes/childscene.rawscene"
  (list
    ;(list "(dialog" "anchor" ".PARENT/)menuitem_0")
  )
  "testname"

)

(define currsceneid (list-sceneid (gameobj-id mainobj)))
(define (onMouse button action mods)
  (define testnameSceneId (lsscene-name "testname"))
  (define testnameSceneIdRoot (if testnameSceneId (scene-rootid testnameSceneId) #f))
  (define currRoot (scene-rootid currsceneid))
  (if (and testnameSceneId currRoot) (make-parent testnameSceneIdRoot currRoot))
  ;(if testnameSceneId (unload-scene testnameSceneId))

  (format #t "lsobj script: scene id: ~a\n" currsceneid)
  (format #t "parent scene id: ~a\n" (parent-scene currsceneid))
  (format #t "child scene id: ~a\n" (child-scenes currsceneid))
  (format #t "lsobj script: BASIC: id is: ~a\n" (gameobj-id (lsobj-name "someotherobject")))
  (format #t "loaded scene id is: ~a\n" (number->string testnameSceneId))
  (format #t "root id is: ~a\n" (number->string testnameSceneIdRoot))
)

