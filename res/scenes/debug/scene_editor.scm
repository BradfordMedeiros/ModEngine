
(define allscenes (list 
  "./res/scenes/features/lighting/lights.rawscene" 
  "./res/scenes/features/lighting/tint.rawscene" 

))
(define activescene "./res/scenes/features/lighting/lights.rawscene")
(define loadedSceneId #f)

(define (findIndex scenes activescene currIndex) 
  (if (equal? (length scenes) 0)
    #f
    (if (equal? (car scenes) activescene)
      currIndex
      (findIndex (cdr scenes) activescene (+ currIndex 1))
    )
  )
)
(define (getCurrSceneIndex) (findIndex allscenes activescene 0))

(define (updateSceneText scenename)
  (gameobj-setattr! 
    (lsobj-name ")current_scene") 
    (list
      (list "value" (string-pad scenename 15))
    )
  )
)

(define (changeScene scenename)
  (display (string-append "change scene to: " scenename "\n"))
  (updateSceneText scenename)
  (set! activescene scenename)
  (if (not (equal? loadedSceneId #f))
    (unload-scene loadedSceneId)
    (set! loadedSceneId #f)
  )
  (set! loadedSceneId (load-scene scenename))
)

(define (prevSceneName) (list-ref allscenes (max (- (getCurrSceneIndex) 1) 0)))
(define (nextSceneName) (list-ref allscenes (min (+ (getCurrSceneIndex) 1) (- (length allscenes) 1))))

(define (changeScenePrev) (changeScene (prevSceneName)))
(define (changeSceneNext) (changeScene (nextSceneName)))


(define (onObjSelected obj color)
  (define objname (gameobj-name obj))
  (cond
    ((equal? objname "*prev_scene"    ) (changeScenePrev))
    ((equal? objname "*next_scene"    ) (changeSceneNext))
    ((equal? objname ")current_scene" ) (updateSceneText (string-append "text-" (number->string (random 10)))))
  )
)

(changeScene activescene)