

(define (onObjHover id)
  (set-state "paint_on")
)

(define (onObjUnhover id)
  (display "on object unhover: ")
  (display id)
  (display "\n")
  (set-state "paint_off")
)

;; set-fstate
; opacity
; drawsize
; drawcolor-r
; drawcolor-g
; drawcolor-b

