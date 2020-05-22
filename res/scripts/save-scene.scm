
(define lastSaveTime 0)
(define saveTimeInSeconds 10)

(define (onFrame)
  (define current-time (time-seconds))
  (if (< saveTimeInSeconds (- current-time lastSaveTime)) 
    (begin
      (save-scene)
      (set! lastSaveTime current-time)
      (display "SCRIPT - save-scene.SCM - saving scene")
    )
  )
)