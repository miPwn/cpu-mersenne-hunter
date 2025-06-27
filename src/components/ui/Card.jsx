import React from 'react'
import { motion } from 'framer-motion'
import clsx from 'clsx'

export function Card({ 
  children, 
  className, 
  hover = true, 
  animate = true,
  ...props 
}) {
  const Component = animate ? motion.div : 'div'
  
  const motionProps = animate ? {
    initial: { opacity: 0, y: 20 },
    animate: { opacity: 1, y: 0 },
    transition: { duration: 0.3 }
  } : {}

  return (
    <Component
      className={clsx(
        'card',
        hover && 'hover:shadow-lg dark:hover:shadow-2xl',
        className
      )}
      {...motionProps}
      {...props}
    >
      {children}
    </Component>
  )
}

export function CardHeader({ children, className, ...props }) {
  return (
    <div 
      className={clsx(
        'p-6 pb-4',
        className
      )}
      {...props}
    >
      {children}
    </div>
  )
}

export function CardContent({ children, className, ...props }) {
  return (
    <div 
      className={clsx(
        'p-6 pt-0',
        className
      )}
      {...props}
    >
      {children}
    </div>
  )
}

export function CardTitle({ children, className, ...props }) {
  return (
    <h3 
      className={clsx(
        'text-lg font-semibold text-gray-900 dark:text-gray-100',
        className
      )}
      {...props}
    >
      {children}
    </h3>
  )
}