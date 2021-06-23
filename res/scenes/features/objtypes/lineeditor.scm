(define script-unique-prefix (number->string (random 1000000000)))
(define prefix-name (string-append "lineeditor-" script-unique-prefix))
(define script-managed-objs (list))

(define (parse-point string-point)
  (define point (map string->number (string-split string-point #\ )))
  (list (car point) (cadr point) (caddr point))
)
(define (parse-points points) (map parse-point (string-split points #\|)))

(define (points-from-line) 
  (parse-points (cadr (assoc "points" (gameobj-attr mainobj))))
)
(define (managed-point-name point-number) (string-append script-unique-prefix "-" (number->string point-number)))
(define (ensure-single-point-obj-exist point-name)
  (define obj (lsobj-name point-name))
  (if (not obj) (mk-obj-attr point-name (list)))
  point-name
)
(define (ensure-point-obj-exist num-points) (map ensure-single-point-obj-exist (map managed-point-name (iota num-points))))
(define (update-point-obj-pos point obj-name) (gameobj-setattr! (lsobj-name obj-name) (list (list "position" point))))
(define (update-point-obj-positions points managed-obj-names) 
  (map 
    (lambda (pointIndex) (update-point-obj-pos (list-ref points pointIndex) (list-ref managed-obj-names pointIndex))) 
    (iota (length points))
  )
)
(define (ensure-point-objs) 
  (define points (points-from-line))
  (set! script-managed-objs (ensure-point-obj-exist (length points)))
  (update-point-obj-positions points script-managed-objs)
)

(define (delete-point point-name) (display "delete obj placeholder\n"))
(define (delete-managed-points) (map delete-point script-managed-objs))

(define (point-from-single-gameobj gameobj) 
  (cadr (assoc "position" (gameobj-attr (lsobj-name gameobj))))
)
(define (points-from-gameobj) (map point-from-single-gameobj script-managed-objs))
(define (update-line) 
  (gameobj-setattr! 
    mainobj
    (list 
      (list "points" (string-join (points-from-gameobj) "|"))
    )
  )
  (genmesh 
    (list
      (list 0 0 0)
      (list 1 0 0)
      (list 0.5 1 0)
    ) 
    (map parse-point (points-from-gameobj))
    "testmesh"
  )
)

(define (onKeyChar key)
  (if (equal? key 91) (ensure-point-objs))
  (if (equal? key 93) (update-line))
)

(define (beforeUnload) (delete-managed-points))


