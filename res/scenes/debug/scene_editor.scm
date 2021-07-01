
(define (changeScene scenename)
  (display (string-append "change scene to: " scenename "\n"))
)

(define (prevSceneName) "placeholder_previous")
(define (nextSceneName) "placeholder_next")

(define (changeScenePrev) (changeScene (prevSceneName)))
(define (changeSceneNext) (changeScene (nextSceneName)))

(define (updateSceneText scenename)
  (display "update scene text to: ")
  (display scenename)
  (display "\n")
  (gameobj-setattr! 
    (lsobj-name ")current_scene") 
    (list
      (list "value" (string-pad scenename 15))
    )
  )
)

(define (onObjSelected obj color)
  (define objname (gameobj-name obj))
  (cond
    ((equal? objname "*prev_scene"    ) (changeScenePrev))
    ((equal? objname "*next_scene"    ) (changeSceneNext))
    ((equal? objname ")current_scene" ) (updateSceneText (string-append "text-" (number->string (random 10)))))
  )
)