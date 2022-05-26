
(define meshObj (lsobj-name "1/Plane"))

(define lasttime (time-seconds))
(define (time-elapsed amount) 
  (> (- (time-seconds) lasttime) amount)
)

(define xpos 0)
(define (onFrame)
  ;(format #t "scrolling on frame called!\n")
  ;(format #t "x pos: ~a\n" xpos)
  (if (time-elapsed 0.01)
    (begin
      (set! lasttime (time-seconds))
      (set! xpos (+ xpos 0.001))
      (if (> xpos 1) (set! xpos 0))
      ;(format #t "setting texture offset: ~a\n" (string-append (number->string xpos) " 0"))
      (gameobj-setattr! 
        meshObj
        (list
          (list "textureoffset" (string-append (number->string xpos) " 0"))
        )
      )
      (gameobj-setattr! 
        meshObj
        (list
          (list "color" (list (* 10 xpos) 1 1 1))
        )
      )
    )
  ) 
)