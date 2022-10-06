
(define lastSaveTime 0)
(define saveTimeInSeconds 10)

(define (scenesToSave)
  (define scenes (list-scenes (list "save")))
  scenes
)

(define (save sceneId)
  (format #t "save scene: ~a\n" sceneId)
  (save-scene #f sceneId)
)

(define (onFrame)
  (define current-time (time-seconds #t))
  (if (< saveTimeInSeconds (- current-time lastSaveTime)) 
    (begin
      (for-each save (scenesToSave))
      (set! lastSaveTime current-time)
      (display "SCRIPT - save-scene.SCM - saving scene")
    )
  )
)