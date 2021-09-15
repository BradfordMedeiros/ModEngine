
; draws a circle

(define (onFrame)
  (define x (cos (time-seconds)))
  (define y (sin (time-seconds)))
  (draw-line (list 0 0 0) (list x 1 y))
)