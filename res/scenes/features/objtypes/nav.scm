
(define camera (lsobj-name ">uicamera"))

(define (pos-from-collision coll) (cadr coll))
(define (distance point1 point2)
  (let ((diff1 (- (car point1) (car point2))) (diff2 (- (cadr point1) (cadr point2))) (diff3 (- (caddr point1) (caddr point2))) )
    (sqrt (+ (* diff1 diff1) (* diff2 diff2) (* diff3 diff3)))
  )
)
(define (closest-hitpoint collisions basepoint)
  (define distances (map (lambda(point) (distance point basepoint)) collisions))
  (define minvalue (apply min distances))
  (define minindex (list-index distances minvalue))
  (define closestpoint (list-ref collisions minindex))
  closestpoint
)

(define (ai-nav)
  (define frompos (gameobj-pos camera))
  (define ray (raycast frompos (gameobj-rot camera) 500))
  (if (> (length ray) 0)
    (gameobj-setpos! 
      mainobj 
      (navpos (gameobj-id mainobj) (closest-hitpoint (map pos-from-collision ray) frompos))
    )
    (display "raycast is 0!!!!\n")
  )
)

(define (onMouse button action mods)
  (ai-nav)
)

(define (onFrame)
  (ai-nav)
)
