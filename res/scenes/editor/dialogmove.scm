
"onObjSelected";onObjHover

(define (onObjSelected obj color)
  (format #t "on obj selected\n")
)


(define (onObjHover obj)
  (format #t "on obj hover\n")
)

(define (onMouseMove xpos ypos mousexNdi mouseyNdi)
  (define offset (list (* xpos 0.05) (* ypos 0.05) 0))
  (format #t "offset is: ~a\n" offset)
  (gameobj-setpos-rel! mainobj offset)
)