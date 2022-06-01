
(define modelpath (if (args "modelpath") (args "modelpath") "./res/models/electricbox/electricbox.obj"))
(format #t "Growing objs: using model: ~a\n" modelpath)
(define rate (if (args "rate") (string->number (args "rate")) 1))

(define objid 0)
(define (make-box)
  (mk-obj-attr (string-append "obj-" (number->string objid)) (list
    (list "mesh"     modelpath)
    (list "position" (list (modulo objid 100) (quotient objid 100) 0))
  ))
  (set! objid (+ objid 2))
)

(define last-time 0)
(define (onFrame)
  (define now (time-seconds))
  (if (> (- now last-time) rate)
    (begin
      (set! last-time now)
      (make-box)
    )
  )
)
