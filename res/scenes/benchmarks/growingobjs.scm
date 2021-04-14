
(define objid 0)
(define (make-box)
  (mk-obj-attr (string-append "obj-" (number->string objid)) (list
    (list "mesh"     "./res/models/electricbox/electricbox.obj")
    (list "position" (list (modulo objid 100) (quotient objid 100) 0))
  ))
  (set! objid (+ objid 2))
)

(define last-time 0)
(define (onFrame)
  (define now (time-seconds))
  (if (> (- now last-time) 1)
    (begin
      (set! last-time now)
      (make-box)
    )
  )
)
