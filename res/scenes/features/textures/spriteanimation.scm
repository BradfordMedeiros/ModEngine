(set! mainobj (lsobj-name "1/Plane"))

(define lasttime (time-seconds))
(define (time-elapsed amount) 
  (> (- (time-seconds) lasttime) amount)
)

(define xpos 0)
(define (onFrame)
  (if (time-elapsed 0.1)
    (begin
      (set! lasttime (time-seconds))
      (set! xpos (+ xpos 0.125))
      (if (> xpos 1) (set! xpos 0))
      (gameobj-setattr! 
        mainobj
        (list
          (list "textureoffset" (string-join (list (number->string xpos) "0") " "))
        )
      )
    )
  ) 
)