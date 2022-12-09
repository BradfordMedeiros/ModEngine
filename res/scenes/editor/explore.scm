
(define hoveredObj #f)
(define (onObjHover obj)
  (set! hoveredObj obj)
)
(define (onObjUnhover index)
  (set! hoveredObj #f)
)

(define (handleAttr onclick)
	(define onclickValue (if onclick (cadr onclick) #f))
	(if onclickValue (sendnotify "explorer" onclickValue))
)
(define (onMouse button action mods)
  (if (and (equal? button 0) (equal? action 0) (and hoveredObj))
    (if (equal? (list-sceneid (gameobj-id hoveredObj)) (list-sceneid (gameobj-id mainobj)))
      (handleAttr (assoc "onclick" (gameobj-attr hoveredObj)))
    )
  )
)


(define (onMessage key value)
	(format #t "on message: ~a ~a\n" key value)
)