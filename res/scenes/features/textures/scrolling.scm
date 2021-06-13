
(define meshObj (lsobj-name "1/Plane"))

(define lasttime (time-seconds))
(define (time-elapsed amount) 
  (> (- (time-seconds) lasttime) amount)
)

(define xpos 0)
(define (onFrame)
  (if (time-elapsed 0.01)
    (begin
      (set! lasttime (time-seconds))
      (set! xpos (+ xpos 0.01))
      (if (> xpos 1) (set! xpos 0))
      (gameobj-setattr! 
        mainobj
        (list
          (list "position" (list xpos 0 0))
        )
      )
      (gameobj-setattr! 
        meshObj
        (list
          (list "color" (list (* 10 xpos) 1 1))
        )
      )
    )
  ) 
)